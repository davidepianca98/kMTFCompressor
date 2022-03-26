
#include <array>
#include <boost/dynamic_bitset.hpp>
#include <thread>
#include <boost/multiprecision/cpp_int.hpp>
#include "MTFHashTableStream.h"
#include "Core.h"
#include "RabinKarp.h"
#include "bitstream.h"
#include "encoders/AdaptiveEliasGamma.h"
#include "encoders/AdaptiveHuffman.h"

template <typename T>
MTFHashTableStream<T>::MTFHashTableStream(int k, int blockSize, Hash& hash) : MTFHashTable<T>(k, blockSize, hash), stop_thread(false) {
    num_cores = 1;//Core::get_cores() - 4;
    read_bytes = 0;
    bytes_to_write = 0;
    in_data.resize(this->block_size);
    mtf_out_data.resize(this->block_size + 1000 * 1024);
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
                this->count_symbol_out(mtf_out_data[i]); // TODO works with only one coder
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
            Core::compress_final(mtf_out_data.data(), bytes_to_write, reinterpret_cast<uint32_t *>(out_data), compressed_size);

            out.write(reinterpret_cast<const char *>(&compressed_size), 4);
            out.write(reinterpret_cast<const char *>(out_data), (long) compressed_size);
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

    std::vector<uint8_t> start(this->hash_function.get_window_size());
    uint8_t c;
    int i;
    for (i = 0; i < start.size() && in.good(); i++) {
        in.read(reinterpret_cast<char *>(&c), 1);
        in_data[i] = c;
        start[i] = c;
        this->count_symbol_in(c);
        mtf_out_data[i] = (uint32_t) c + this->byte_size();
        this->count_symbol_out(mtf_out_data[i]);
    }
    if (!in.good()) {
        throw std::runtime_error("Not enough data to run the algorithm");
    }
    read_bytes = i;

    this->hash_function.init(start);

    while (read_bytes > 0) {
        mon.wait(Monitor::READER);
        //std::cout << "READING" << std::endl;

        // Double the table size if necessary, before calculating next hashes
        this->double_table();

        in.read(reinterpret_cast<char *>(in_data.data() + i), this->block_size - i);
        read_bytes = in.gcount() + i;

        start_hash = i;
        while (i < read_bytes) {
            c = in_data[i];
            this->count_symbol_in(c);

            // Prepare hash array to allow threads to use their part of the table in parallel
            hashes[i] = this->hash_function.get_hash();

            this->hash_function.update(c);
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
}




void compress(int bytes, std::ostream& out, uint32_t *in_data, uint32_t *out_data) {
    size_t compressed_size = bytes * 4 + 1024;
    Core::compress_final(in_data, bytes, out_data, compressed_size);

    out.write(reinterpret_cast<const char *>(&compressed_size), 4);
    out.write(reinterpret_cast<const char *>(out_data), (long) compressed_size);
}

template <typename T>
void MTFHashTableStream<T>::encode2(std::istream& in, std::ostream& out) {
    std::vector<uint8_t> start(this->hash_function.get_window_size());
    auto *out_data = new uint32_t[this->block_size + 100 * 1024];

    bool rle = false;

    uint8_t c;
    int i;
    int out_i = 0;
    for (i = 0; i < start.size() && in.good(); i++, out_i++) {
        in.read(reinterpret_cast<char *>(&c), 1);
        in_data[i] = c;
        start[i] = c;
        this->count_symbol_in(c);
        mtf_out_data[out_i] = (uint32_t) c + this->byte_size();
        this->count_symbol_out(mtf_out_data[out_i]);
    }
    if (!in.good()) {
        throw std::runtime_error("Not enough data to run the algorithm");
    }
    read_bytes = i;

    this->hash_function.init(start);

    auto *mtf_out_data2 = new uint32_t[this->block_size + 100 * 1024];

    std::future<void> future;

    while (read_bytes > 0) {
        // Read block
        in.read(reinterpret_cast<char *>(in_data.data() + i), this->block_size - i);
        read_bytes = in.gcount() + i;

        auto *run_counter = reinterpret_cast<int8_t *>(&mtf_out_data[out_i]); // TODO rle also in initialization
        uint32_t last_c = UINT32_MAX; // TODO inizialize correctly
        if (rle && read_bytes > 0) {
            out_i++;
            *run_counter = 0;
        }

        int counter = 0;

        // Apply transformation
        while (i < read_bytes) {
            uint32_t out_c = this->mtfEncode(in_data[i]);
            if (rle) {
                if (out_c == last_c) { // TODO should check that when counter is == max or min we need to start another counter
                    counter++;
                    if (*run_counter < 0 && counter >= 2) {
                        (*run_counter) += 2;
                        run_counter = reinterpret_cast<int8_t *>(&mtf_out_data[out_i - 2]);
                        *run_counter = 3;
                    } else {
                        (*run_counter)++;
                    }
                } else {
                    if (*run_counter > 0) {
                        run_counter = reinterpret_cast<int8_t *>(&mtf_out_data[out_i++]);
                        *run_counter = -1;
                    } else {
                        (*run_counter)--;
                    }
                    mtf_out_data[out_i++] = out_c;
                    last_c = out_c;
                    counter = 1;
                }
            } else {
                mtf_out_data[out_i++] = out_c;
                //std::cout << out_c << " ";
            }
            this->double_table();
            i++;
        }
        i = 0;

        if (future.valid()) {
            future.wait();
        }

        if (read_bytes > 0) {
            memcpy(mtf_out_data2, mtf_out_data.data(), out_i * 4);
            future = std::async(std::launch::async, compress, out_i, std::ref(out), mtf_out_data2, out_data);
        }
        out_i = 0;
    }
    future.wait();

    this->print_stats();
    delete[] out_data;
    delete[] mtf_out_data2;
}

template <typename T>
void MTFHashTableStream<T>::decompress(int decompressed_size, std::ostream& out, const uint32_t *in) {
    std::vector<uint8_t> start(this->hash_function.get_window_size());

    for (int i = 0; i < decompressed_size; i++) {
        if (!started && i < start.size()) {
            start[i] = (uint8_t) (in[i] - this->byte_size());
            in_data[i] = start[i];
            if (i == start.size() - 1) {
                started = true;
                this->hash_function.init(start);
            }
        } else {
            in_data[i] = this->mtfDecode(in[i]);
            this->double_table();
        }
    }
    out.write(reinterpret_cast<const char *>(in_data.data()), (long) decompressed_size);
}

template <typename T>
void MTFHashTableStream<T>::decode2(std::istream &in, std::ostream &out) {
    started = false;

    auto *out_block1 = new uint32_t[this->block_size];
    auto *out_block2 = new uint32_t[this->block_size];

    long read_bytes_amount;
    std::future<void> future;
    do {
        uint32_t block_size;
        in.read(reinterpret_cast<char *>(&block_size), 4);
        in.read(reinterpret_cast<char *>(mtf_out_data.data()), block_size);
        read_bytes_amount = in.gcount();
        if (read_bytes_amount > 0) {
            size_t decompressed_size = mtf_out_data[0] * 4 * 32 + 1024;

            Core::decompress_final(reinterpret_cast<const uint32_t *>(mtf_out_data.data()), read_bytes_amount / 4, out_block1, decompressed_size);

            if (future.valid()) {
                future.wait();
            }

            memcpy(out_block2, out_block1, decompressed_size * 4);
            future = std::async(std::launch::async, &MTFHashTableStream::decompress, this, decompressed_size, std::ref(out), out_block2);
        }
    } while (read_bytes_amount > 0);
    future.wait();

    delete[] out_block1;
    delete[] out_block2;
}





void entropy_rle_encode(const uint32_t *data, int bytes, AdaptiveEliasGamma& aeg, AdaptiveHuffman& ah, obitstream& out) {
    if (bytes > 0) {
        uint32_t last = data[0];
        uint64_t counter = 1;
        for (int i = 1; i < bytes; i++) {
            if (last == data[i]) {
                counter++;
            } else {
                aeg.encode(counter, out);
                ah.encode(last, out);
                last = data[i];
                counter = 1;
            }
        }
        aeg.encode(counter, out);
        ah.encode(last, out);

        std::cout << "|" << std::flush;
    }
}

void entropy_encode(const uint32_t *data, int bytes, AdaptiveHuffman& ah, obitstream& out) {
    for (int i = 0; i < bytes; i++) {
        ah.encode(data[i], out);
    }
    std::cout << "|" << std::flush;
}

template <typename T>
void MTFHashTableStream<T>::encode(std::istream& in, obitstream& out) {
    started = false;
    std::vector<uint8_t> start(this->hash_function.get_window_size());
    std::future<void> future;
    auto *out_block1 = new uint32_t[this->block_size];

    AdaptiveEliasGamma aeg(UINT16_MAX); // 256 + this->byte_size() for normal
    AdaptiveHuffman ah(this->byte_size());
    do {
        // Read block
        in.read(reinterpret_cast<char *>(in_data.data()), this->block_size);
        read_bytes = in.gcount();

        // Apply transformation
        for (int i = 0; i < read_bytes; i++) {
            uint32_t out_c;
            if (!started && i < start.size()) {
                uint8_t c = in_data[i];
                start[i] = c;
                this->count_symbol_in(c);
                out_c = (uint32_t) c + this->byte_size();
                this->count_symbol_out(out_c);
                if (i == start.size() - 1) {
                    started = true;
                    this->hash_function.init(start);
                }
            } else {
                out_c = this->mtfEncode(in_data[i]);
            }
            mtf_out_data[i] = out_c;

            this->double_table();
        }

        if (future.valid()) {
            future.wait();
        }

        memcpy(out_block1, mtf_out_data.data(), read_bytes * 4);
        //future = std::async(std::launch::async, entropy_rle_encode, out_block1, read_bytes, std::ref(aeg), std::ref(ah), std::ref(out));
        future = std::async(std::launch::async, entropy_encode, out_block1, read_bytes, std::ref(ah), std::ref(out));

    } while (read_bytes > 0);
    if (future.valid()) {
        future.wait();
    }
    out.flush();

    std::cout << std::endl;
    this->print_stats();
    delete[] out_block1;
}

template <typename T>
void MTFHashTableStream<T>::decode(ibitstream &in, std::ostream &out) {
    started = false;

    std::vector<uint8_t> start(this->hash_function.get_window_size());

    AdaptiveHuffman ah(this->byte_size());
    int i = 0;
    while (in.remaining()) {
        uint32_t num = ah.decode(in);
        if (num < 0) {
            break;
        }

        if (!started && i < start.size()) {
            start[i] = (uint8_t) (num - this->byte_size());
            out.put((char) start[i]);
            if (i == start.size() - 1) {
                started = true;
                this->hash_function.init(start);
            }
        } else {
            out.put(this->mtfDecode(num));
            this->double_table();
        }
        i++;
    }
}

template class MTFHashTableStream<uint16_t>;
template class MTFHashTableStream<uint32_t>;
template class MTFHashTableStream<uint64_t>;
template class MTFHashTableStream<boost::multiprecision::uint128_t>;
template class MTFHashTableStream<boost::multiprecision::uint256_t>;
template class MTFHashTableStream<boost::multiprecision::uint512_t>;
template class MTFHashTableStream<boost::multiprecision::uint1024_t>;
