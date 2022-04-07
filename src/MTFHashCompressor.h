
#ifndef MTF_MTFHASHCOMPRESSOR_H
#define MTF_MTFHASHCOMPRESSOR_H

#include <iostream>
#include <fstream>
#include <random>
#include "MTFHashCompressor.h"
#include "MTFBlockWorker.h"
#include "MTFHashTableStream.h"
#include "RabinKarp.h"
#include "Identity.h"
#include "stream/ofbitstream.h"
#include "stream/ifbitstream.h"

class MTFHashCompressor {

public:
    static int get_cores() {
        int processor_count = (int) std::thread::hardware_concurrency();
        if (processor_count == 0) {
            processor_count = 1;
        }
        return processor_count;
    }

    template <typename HASH, typename T>
    static int compress_block(const std::string& path, const std::string& out_path, int k) {
        std::ifstream in_file(path, std::ios::binary);
        if (in_file.fail()) {
            return 1;
        }
        std::ofstream out_file(out_path, std::ios::binary);

        int core_number = get_cores() - 2;

        int block_size = 4 * 1024 * 1024; // 1 MB block size

        std::vector<MTFBlockWorker<HASH, T>> workers;
        for (int i = 0; i < core_number; i++) {
            workers.emplace_back(k, 4096, block_size, block_size * 4 + 1024);
            //workers.emplace_back(k, 256 * 256 * 256, block_size, block_size * 4 + 1024);
        }

        while (in_file.good()) {
            for (MTFBlockWorker<HASH, T>& worker: workers) {
                in_file.read(reinterpret_cast<char *>(worker.get_in_block()), worker.get_in_block_size());
                long read_bytes = in_file.gcount();

                worker.startCompression(read_bytes);
            }

            for (MTFBlockWorker<HASH, T>& worker: workers) {
                uint32_t compressed_block_size = worker.get();
                if (compressed_block_size > 0) {
                    out_file.write(reinterpret_cast<const char *>(&compressed_block_size), 4);
                    out_file.write(reinterpret_cast<const char *>(worker.get_out_block()), compressed_block_size);
                }
            }
        }

        in_file.close();
        out_file.close();

        return 0;
    }

    template <typename HASH, typename T>
    static int decompress_block(const std::string& path, const std::string& out_path, int k) {
        std::ifstream in_file(path, std::ios::binary);
        if (in_file.fail()) {
            return 1;
        }
        std::ofstream out_file(out_path, std::ios::binary);

        int core_number = get_cores() - 2;

        // Needs max block size as the block size is read after the allocation of these buffers
        int max_block_size = 1024 * 1024 * 10;

        std::vector<MTFBlockWorker<HASH, T>> workers;
        for (int i = 0; i < core_number; i++) {
            workers.emplace_back(k, 4096, max_block_size, max_block_size * 4 + 1024);
            //workers.emplace_back(k, 256 * 256 * 256, max_block_size, max_block_size * 4 + 1024);
        }

        while (in_file.good()) {
            for (MTFBlockWorker<HASH, T>& worker: workers) {
                uint32_t block_size;
                in_file.read(reinterpret_cast<char *>(&block_size), 4);

                in_file.read(reinterpret_cast<char *>(worker.get_in_block()), block_size);
                long read_bytes = in_file.gcount();

                worker.startDecompression(read_bytes);
            }

            for (MTFBlockWorker<HASH, T>& worker: workers) {
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

    template <typename HASH, typename T>
    static int compress_stream(const std::string& path, const std::string& out_path, int k) {
        std::ifstream in_file(path, std::ios::binary);
        if (in_file.fail()) {
            return 1;
        }
        ofbitstream out_file(out_path);

        HASH hash(k, 4096);
        //HASH hash(k, 256 * 256 * 256);

        MTFHashTableStream<T> mtf(1024 * 1024, hash); // 1 MB block size
        mtf.encode(in_file, out_file);

        in_file.close();
        out_file.close();

        return 0;
    }

    template <typename HASH, typename T>
    static int decompress_stream(const std::string& path, const std::string& out_path, int k) {
        ifbitstream in_file(path);
        //std::ifstream in_file(path);
        if (in_file.fail()) {
            return 1;
        }
        std::ofstream out_file(out_path, std::ios::binary);

        HASH hash(k, 4096);
        //HASH hash(k, 256 * 256 * 256);
        MTFHashTableStream<T> mtf(1024 * 1024, hash); // 1 MB block size
        mtf.decode(in_file, out_file);

        in_file.close();
        out_file.close();

        return 0;
    }
};


#endif //MTF_MTFHASHCOMPRESSOR_H
