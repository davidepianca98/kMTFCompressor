
#include <thread>
#include "MTFHashTableStream.h"
#include "Core.h"
#include "RabinFingerprint.h"

MTFHashTableStream::MTFHashTableStream(int k, int blockSize) : MTFHashTable(k, blockSize), stop_thread(false) {
    num_cores = Core::get_cores() - 4;
    read_bytes = 0;
    bytes_to_write = 0;
    in_data = new uint8_t[block_size];
    mtf_out_data = new uint32_t[block_size];
    hashes.resize(block_size);
}

void MTFHashTableStream::encoder_thread(int thread_number) {
    double divider = (double) num_cores / (double) hash_table.size();

    while (!stop_thread || read_bytes > 0) {
        mon.wait(Monitor::CODER);
        //std::cout << "ENCODING " << read_bytes << std::endl;
        for (int i = start_hash; i < read_bytes; i++) {
            uint64_t hash = hashes[i];
            int tn = hash * divider;
            if (tn == thread_number) {
                mtf_out_data[i] = mtfEncode(hash_table[hash], in_data[i]);
            }
        }
        bytes_to_write = read_bytes;
        mon.notify(Monitor::CODER);
    }
}

void MTFHashTableStream::writer_thread(std::ostream& out) {
    auto *out_data = new uint8_t[block_size * 4 + 1024];

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

void MTFHashTableStream::encode(std::istream& in, std::ostream& out) {
    stop_thread = false;

    for (int j = 0; j < num_cores; j++) {
        threads.emplace_back(&MTFHashTableStream::encoder_thread, this, j);
    }
    writer = std::thread(&MTFHashTableStream::writer_thread, this, std::ref(out));

    std::vector<uint8_t> start(k);
    uint8_t c;
    int i;
    for (i = 0; i < k; i++) { // TODO incorporate in mtfEncode
        in.read(reinterpret_cast<char *>(&c), 1);
        in_data[i] = c;
        start[i] = c;
        mtf_out_data[i] = (uint32_t) c + 8;
    }
    read_bytes = k;

    // Rolling hash to access the table
    RabinFingerprint hash(k, start);

    while (read_bytes > 0) {
        mon.wait(Monitor::READER);
        //std::cout << "READING" << std::endl;

        in.read(reinterpret_cast<char *>(in_data + i), block_size - i);
        read_bytes = in.gcount() + i;

        start_hash = i;
        while (i < read_bytes) {
            c = in_data[i];
            keep_track(hash.get_hash());

            hashes[i] = hash.get_hash();

            hash.update(c);
            i++;
        }
        i = 0;

        mon.notify(Monitor::READER);
    }

    //print_stats();

    stop();
}

void MTFHashTableStream::stop() {
    stop_thread = true;
    for (auto & thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    if (writer.joinable()) {
        writer.join();
    }
}

MTFHashTableStream::~MTFHashTableStream() {
    stop();
    delete[] in_data;
    delete[] mtf_out_data;
}

void MTFHashTableStream::decode(std::istream &in, std::ostream &out) { // TODO parallelize
    uint32_t c;

    uint32_t block_size;
    in.read(reinterpret_cast<char *>(&block_size), 4);
    in.read(reinterpret_cast<char *>(mtf_out_data), block_size);
    long read_bytes = in.gcount();
    if (read_bytes < k) {
        throw std::runtime_error("Not enough data to read");
    }

    size_t decompressed_size = mtf_out_data[0] * 4 * 32 + 1024; // TODO probably move in final
    uint32_t out_block1[this->block_size];
    Core::decompress_final(mtf_out_data, read_bytes / 4, out_block1, decompressed_size);



    std::vector<uint8_t> start(k);
    int i;
    for (i = 0; i < k; i++) { // TODO incorporate in mtfDecode
        c = out_block1[i];
        start[i] = (uint8_t) (c - 8);
        in_data[i] = start[i];
    }

    // Rolling hash to access the table
    RabinFingerprint hash(k, start);

    do {

        while (i < decompressed_size) {
            c = out_block1[i];
            uint8_t ca = mtfDecode(hash_table[hash.get_hash()], c);
            in_data[i] = ca;

            hash.update(ca);
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
