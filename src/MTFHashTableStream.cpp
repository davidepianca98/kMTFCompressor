
#include <thread>
#include <boost/multiprecision/cpp_int.hpp>
#include "MTFHashTableStream.h"
#include "Core.h"
#include "RabinFingerprint.h"

template <typename T>
MTFHashTableStream<T>::MTFHashTableStream(int k, int blockSize, Hash& hash) : MTFHashTable<T>(k, blockSize, hash), stop_thread(false) {
    num_cores = 1;//Core::get_cores() - 4;
    read_bytes = 0;
    bytes_to_write = 0;
    in_data = new uint8_t[this->block_size];
    mtf_out_data = new uint32_t[this->block_size];
    hashes.resize(this->block_size);
}

template <typename T>
void MTFHashTableStream<T>::encoder_thread(int thread_number) {
    double divider = (double) num_cores / (double) this->hash_table.size();

    while (!stop_thread || read_bytes > 0) {
        mon.wait(Monitor::CODER);
        //std::cout << "ENCODING " << read_bytes << std::endl;
        for (int i = start_hash; i < read_bytes; i++) {
            uint64_t hash = hashes[i];
            int tn = hash * divider;
            if (tn == thread_number) {
                //mtf_out_data[i] = this->mtfEncode(this->hash_table[hash], in_data[i]); TODO after changes for variable size table no longer possible
            }
        }
        bytes_to_write = read_bytes;
        mon.notify(Monitor::CODER);
    }
}

template <typename T>
void MTFHashTableStream<T>::writer_thread(std::ostream& out) {
    auto *out_data = new uint8_t[this->block_size * 4 + 1024];

    while (!stop_thread || bytes_to_write > 0) {
        mon.wait(Monitor::WRITER);
        //std::cout << "WRITING " << bytes_to_write << std::endl;
        if (bytes_to_write > 0) {
            size_t compressed_size = bytes_to_write * 4 + 1024;
            Core::compress_final(mtf_out_data, bytes_to_write, reinterpret_cast<uint32_t *>(out_data), compressed_size);

            out.write(reinterpret_cast<const char *>(&compressed_size), 4);
            out.write(reinterpret_cast<const char *>(out_data), compressed_size);
        }
        mon.notify(Monitor::WRITER);
    }

    delete[] out_data;
}

template <typename T>
void MTFHashTableStream<T>::encode_pipeline(std::istream& in, std::ostream& out) {
    this->stop_thread = false;

    for (int j = 0; j < num_cores; j++) {
        threads.emplace_back(&MTFHashTableStream::encoder_thread, this, j);
    }
    this->writer = std::thread(&MTFHashTableStream::writer_thread, this, std::ref(out));

    std::vector<uint8_t> start(this->k);
    uint8_t c;
    int i;
    for (i = 0; i < this->k; i++) { // TODO incorporate in mtfEncode
        in.read(reinterpret_cast<char *>(&c), 1);
        in_data[i] = c;
        start[i] = c;
        mtf_out_data[i] = (uint32_t) c + 8;
    }
    read_bytes = this->k;

    // Rolling hash to access the table
    RabinFingerprint hash(this->k);
    hash.init(start);

    while (read_bytes > 0) {
        mon.wait(Monitor::READER);
        //std::cout << "READING" << std::endl;

        in.read(reinterpret_cast<char *>(in_data + i), this->block_size - i);
        read_bytes = in.gcount() + i;

        start_hash = i;
        while (i < read_bytes) {
            c = in_data[i];
            this->keep_track(hash.get_hash());

            // Prepare hash array to allow threads to use their part of the table in parallel
            hashes[i] = hash.get_hash();

            hash.update(c);
            i++;
        }
        i = 0;

        mon.notify(Monitor::READER);
    }

    this->print_stats();

    stop();
}

template <typename T>
void MTFHashTableStream<T>::stop() {
    this->stop_thread = true;
    for (auto & thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    if (writer.joinable()) {
        writer.join();
    }
}

template <typename T>
MTFHashTableStream<T>::~MTFHashTableStream() {
    stop();
    delete[] in_data;
    delete[] mtf_out_data;
}

void compress(int bytes, std::ostream& out, uint32_t *in_data, uint8_t *out_data) {
    size_t compressed_size = bytes * 4 + 1024;
    Core::compress_final(in_data, bytes, reinterpret_cast<uint32_t *>(out_data), compressed_size);

    out.write(reinterpret_cast<const char *>(&compressed_size), 4);
    out.write(reinterpret_cast<const char *>(out_data), compressed_size);
}

template <typename T>
void MTFHashTableStream<T>::encode(std::istream& in, std::ostream& out) {
    std::vector<uint8_t> start(15); // TODO window size
    auto *out_data = new uint8_t[this->block_size * 4 + 1024];
    uint8_t c;
    int i;
    for (i = 0; i < start.size(); i++) {
        in.read(reinterpret_cast<char *>(&c), 1);
        in_data[i] = c;
        start[i] = c;
        mtf_out_data[i] = (uint32_t) c + 8;
    }
    read_bytes = i;

    this->hash_function.init(start);

    auto *mtf_out_data2 = new uint32_t[this->block_size];

    std::future<void> future;

    while (read_bytes > 0) {

        in.read(reinterpret_cast<char *>(in_data + i), this->block_size - i);
        read_bytes = in.gcount() + i;

        while (i < read_bytes) {
            mtf_out_data[i] = this->mtfEncode(in_data[i]);
            //std::cout << mtf_out_data[i] << " ";
            i++;
        }
        i = 0;

        //if (future.valid()) {
        //    future.wait();
        //}

        if (read_bytes > 0) {
            //memcpy(mtf_out_data2, mtf_out_data, read_bytes);

            size_t compressed_size = read_bytes * 4 + 1024;
            Core::compress_final(mtf_out_data, read_bytes, reinterpret_cast<uint32_t *>(out_data), compressed_size);

            out.write(reinterpret_cast<const char *>(&compressed_size), 4);
            out.write(reinterpret_cast<const char *>(out_data), compressed_size);
            //future = std::async(std::launch::async, compress, read_bytes, std::ref(out), mtf_out_data2, out_data);
        }
    }
    //future.wait();

    this->print_stats();
    delete[] out_data;
    delete[] mtf_out_data2;
}

template <typename T>
void MTFHashTableStream<T>::decode(std::istream &in, std::ostream &out) { // TODO parallelize
    uint32_t c;

    uint32_t block_size;
    in.read(reinterpret_cast<char *>(&block_size), 4);
    in.read(reinterpret_cast<char *>(mtf_out_data), block_size);
    long read_bytes = in.gcount();
    if (read_bytes < this->k) {
        throw std::runtime_error("Not enough data to read");
    }

    size_t decompressed_size = mtf_out_data[0] * 4 * 32 + 1024; // TODO probably move in final
    uint32_t out_block1[this->block_size];
    Core::decompress_final(mtf_out_data, read_bytes / 4, out_block1, decompressed_size);



    std::vector<uint8_t> start(this->k);
    int i;
    for (i = 0; i < this->k; i++) { // TODO incorporate in mtfDecode
        c = out_block1[i];
        start[i] = (uint8_t) (c - 8);
        in_data[i] = start[i];
    }

    this->hash_function.init(start);

    do {

        while (i < decompressed_size) {
            in_data[i] = this->mtfDecode(out_block1[i]);

            i++;
        }
        i = 0;
        out.write(reinterpret_cast<const char *>(in_data), decompressed_size);

        in.read(reinterpret_cast<char *>(&block_size), 4);
        in.read(reinterpret_cast<char *>(mtf_out_data), block_size);
        read_bytes = in.gcount();

        if (read_bytes > 0) {
            decompressed_size = mtf_out_data[0] * 4 * 32 + 1024; // TODO probably move in final
            Core::decompress_final(reinterpret_cast<const uint32_t *>(mtf_out_data), read_bytes / 4, out_block1,
                                   decompressed_size);
        }
    } while (read_bytes > 0);
}

template class MTFHashTableStream<uint64_t>;
template class MTFHashTableStream<boost::multiprecision::uint1024_t>;
