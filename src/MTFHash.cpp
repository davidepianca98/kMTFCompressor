
#include <iostream>

#include <fstream>
#include <random>
#include "MTFHash.h"
#include "Core.h"
#include "MTFHashTableStream.h"
#include "RabinKarp.h"
#include "VectorHash.h"
#include "MinimiserHash.h"
#include "ConcatenatedHash.h"
#include "Fnv1a.h"
#include "Adler32.h"
#include "CRC.h"
#include "Identity.h"

int MTFHash::compress(const std::string& path, const std::string& out_path, int k) {
    std::ifstream in_file(path, std::ios::binary);
    if (in_file.fail()) {
        return 1;
    }
    std::ofstream out_file(out_path, std::ios::binary);

    int core_number = Core::get_cores() - 2;

    //int block_size = 1024 * 1024; // 1 MB block size

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(5, 21);

    std::vector<Core> cores;
    for (int i = 0; i < core_number; i++) {
        int block_size = dist(mt) * 100 * 1024;
        cores.emplace_back(k, block_size, block_size * 4 + 1024);
    }

    while (in_file.good()) {
        for (Core& core: cores) {
            in_file.read(reinterpret_cast<char *>(core.block.data()), core.block.size());
            long read_bytes = in_file.gcount();

            core.startCompression(read_bytes);
        }

        for (Core& core: cores) {
            uint32_t compressed_block_size = core.get();
            if (compressed_block_size > 0) {
                out_file.write(reinterpret_cast<const char *>(&compressed_block_size), 4);
                out_file.write(reinterpret_cast<const char *>(core.out_block.data()), compressed_block_size);
            }
        }
    }

    in_file.close();
    out_file.close();

    return 0;
}


int MTFHash::compress_stream(const std::string& path, const std::string& out_path, int k) {
    std::ifstream in_file(path, std::ios::binary);
    if (in_file.fail()) {
        return 1;
    }
    std::ofstream out_file(out_path, std::ios::binary);

    //MinimiserHash<Fnv1a, Fnv1a, Fnv1a> hash(k, 15, 1024);
    //ConcatenatedHash<Fnv1a, Fnv1a> hash(k, 15, 1024);
    //ConcatenatedHash<RabinKarp, CRC> hash(k, 15, 1024);
    RabinKarp hash(k, 100000007);
    //CRC hash(k, 100000007);
    //Fnv1a hash(k, 1024);
    //Identity hash(1, 256);
    MTFHashTableStream<uint64_t> mtf(k, 1024 * 1024, hash); // 1 MB block size
    mtf.encode(in_file, out_file);

    in_file.close();
    out_file.close();

    return 0;
}


int MTFHash::decompress(const std::string &path, const std::string &out_path, int k) {
    std::ifstream in_file(path, std::ios::binary);
    if (in_file.fail()) {
        return 1;
    }
    std::ofstream out_file(out_path, std::ios::binary);

    int core_number = Core::get_cores() - 2;

    // Needs max block size as the block size is read after the allocation of these buffers
    int max_block_size = 1024 * 1024 * 10;

    std::vector<Core> cores;
    for (int i = 0; i < core_number; i++) {
        cores.emplace_back(k, max_block_size, max_block_size * 4 + 1024);
    }

    while (in_file.good()) {
        for (Core& core: cores) {
            uint32_t block_size;
            in_file.read(reinterpret_cast<char *>(&block_size), 4);

            in_file.read(reinterpret_cast<char *>(core.block.data()), block_size);
            long read_bytes = in_file.gcount(); // TODO check read_byte is equal to block_size

            core.startDecompression(read_bytes);
        }

        for (Core& core: cores) {
            uint32_t decompressed_block_size = core.get();
            if (decompressed_block_size > 0) {
                out_file.write(reinterpret_cast<const char *>(core.out_block.data()), decompressed_block_size);
            }
        }
    }

    in_file.close();
    out_file.close();

    return 0;
}

int MTFHash::decompress_stream(const std::string& path, const std::string& out_path, int k) {
    std::ifstream in_file(path, std::ios::binary);
    if (in_file.fail()) {
        return 1;
    }
    std::ofstream out_file(out_path, std::ios::binary);

    ConcatenatedHash<Fnv1a, Fnv1a> hash(k, 15, 1024);
    //RabinKarp hash(k);
    MTFHashTableStream<uint64_t> mtf(k, 1024 * 1024, hash); // 1 MB block size
    mtf.decode(in_file, out_file);

    in_file.close();
    out_file.close();

    return 0;
}
