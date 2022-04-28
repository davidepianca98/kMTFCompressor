
#ifndef MTF_MTFHASHCOMPRESSOR_H
#define MTF_MTFHASHCOMPRESSOR_H

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include "MTFHashCompressor.h"
#include "mtf/mtftable/MTFBlockWorker.h"
#include "stream/obitstream/ofbitstream.h"
#include "mtf/mtftable/MTFHashTableStream.h"
#include "stream/ibitstream/ifbitstream.h"

class MTFHashCompressor {

    static uint64_t generate_seed() {
        return std::chrono::system_clock::now().time_since_epoch().count();
    }

public:
    static int get_cores() {
        int processor_count = (int) std::thread::hardware_concurrency() - 2;
        if (processor_count <= 0) {
            processor_count = 1;
        }
        return processor_count;
    }

    template <typename BUFFER, uint32_t SIZE>
    static int compress_block(const std::string& path, const std::string& out_path, int k, uint64_t max_memory_usage) {
        std::ifstream in_file(path, std::ios::binary);
        if (in_file.fail()) {
            return 1;
        }
        std::ofstream out_file(out_path, std::ios::binary);

        uint64_t seed = generate_seed();
        uint64_t be_seed = htobe64(seed);
        out_file.write(reinterpret_cast<const char *>(&be_seed), 8);

        int core_number = get_cores() - 2;

        int block_size = 8 * 1024 * 1024; // 4 MB block size

        std::vector<MTFBlockWorker<BUFFER, SIZE>> workers;
        for (int i = 0; i < core_number; i++) {
            workers.emplace_back(k, seed, block_size, block_size * 4 + 1024, max_memory_usage / core_number);
        }

        while (in_file.good()) {
            for (MTFBlockWorker<BUFFER, SIZE>& worker: workers) {
                in_file.read(reinterpret_cast<char *>(worker.get_in_block()), worker.get_in_block_size());
                long read_bytes = in_file.gcount();

                worker.startCompression(read_bytes);
            }

            for (MTFBlockWorker<BUFFER, SIZE>& worker: workers) {
                uint32_t compressed_block_size = worker.get();
                if (compressed_block_size > 0) {
                    uint32_t be_compressed_block_size = htobe32(compressed_block_size);
                    out_file.write(reinterpret_cast<const char *>(&be_compressed_block_size), 4);
                    out_file.write(reinterpret_cast<const char *>(worker.get_out_block()), compressed_block_size);
                }
            }
        }

        in_file.close();
        out_file.close();

        return 0;
    }

    template <typename BUFFER, uint32_t SIZE>
    static int decompress_block(const std::string& path, const std::string& out_path, int k, uint64_t max_memory_usage) {
        std::ifstream in_file(path, std::ios::binary);
        if (in_file.fail()) {
            return 1;
        }
        std::ofstream out_file(out_path, std::ios::binary);

        uint64_t be_seed;
        in_file.read(reinterpret_cast<char *>(&be_seed), 8);
        uint64_t seed = be64toh(be_seed);

        int core_number = get_cores();

        // Needs max block size as the block size is read after the allocation of these buffers
        int max_block_size = 1024 * 1024 * 10;

        std::vector<MTFBlockWorker<BUFFER, SIZE>> workers;
        for (int i = 0; i < core_number; i++) {
            workers.emplace_back(k, seed, max_block_size, max_block_size * 4 + 1024, max_memory_usage / core_number);
        }

        while (in_file.good()) {
            for (MTFBlockWorker<BUFFER, SIZE>& worker: workers) {
                uint32_t block_size;
                in_file.read(reinterpret_cast<char *>(&block_size), 4);
                block_size = be32toh(block_size);

                in_file.read(reinterpret_cast<char *>(worker.get_in_block()), block_size);
                long read_bytes = in_file.gcount();

                worker.startDecompression(read_bytes);
            }

            for (MTFBlockWorker<BUFFER, SIZE>& worker: workers) {
                uint32_t decompressed_block_size = worker.get();
                if (decompressed_block_size > 0) {
                    out_file.write(reinterpret_cast<const char *>(worker.get_out_block()), decompressed_block_size);
                }
            }
        }

        in_file.close();
        out_file.close();

        return 0;
    }

    template <typename BUFFER, uint32_t SIZE>
    static int compress_stream(const std::string& path, const std::string& out_path, int k, uint64_t max_memory_usage) {
        std::ifstream in_file(path, std::ios::binary);
        if (in_file.fail()) {
            return 1;
        }
        ofbitstream out_file(out_path);

        uint64_t seed = generate_seed();
        uint64_t be_seed = htobe64(seed);
        out_file.write(reinterpret_cast<const char *>(&be_seed), 8);

        MTFHashTableStream<BUFFER, SIZE> mtf(max_memory_usage, k, seed);
        mtf.encode(in_file, out_file);

        in_file.close();
        out_file.close();

        return 0;
    }

    template <typename BUFFER, uint32_t SIZE>
    static int decompress_stream(const std::string& path, const std::string& out_path, int k, uint64_t max_memory_usage) {
        ifbitstream in_file(path);
        if (in_file.fail()) {
            return 1;
        }
        std::ofstream out_file(out_path, std::ios::binary);

        uint64_t be_seed;
        in_file.read(reinterpret_cast<char *>(&be_seed), 8);
        uint64_t seed = be64toh(be_seed);

        MTFHashTableStream<BUFFER, SIZE> mtf(max_memory_usage, k, seed);
        mtf.decode(in_file, out_file);

        in_file.close();
        out_file.close();

        return 0;
    }
};


#endif //MTF_MTFHASHCOMPRESSOR_H
