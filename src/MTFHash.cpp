
#include <iostream>

#include <fstream>
#include "MTFHash.h"
#include "Core.h"
#include "MTFHashTableStream.h"
#include "RabinFingerprint.h"
#include "VectorHash.h"
#include "MinimiserHash.h"

int MTFHash::compress(const std::string& path, const std::string& out_path, int k) {
    std::ifstream in_file(path, std::ios::binary);
    if (in_file.fail()) {
        return 1;
    }
    std::ofstream out_file(out_path, std::ios::binary);

    int core_number = 1;//Core::get_cores() - 1;

    int block_size = 1024 * 1024; // 1 MB block size

    RabinFingerprint hash(k);

    std::vector<Core> cores;
    for (int i = 0; i < core_number; i++) {
        cores.emplace_back(k, block_size, block_size * 4 + 1024, hash);
    }

    while (in_file.good()) {
        for (Core& core: cores) {
            in_file.read(reinterpret_cast<char *>(core.block.data()), core.block.size());
            long read_bytes = in_file.gcount();

            // TODO spawn threads that compress, maybe spawn with different k, and save in block header the best k and the compressed data
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

    MinimiserHash hash(k, 15);
    //RabinFingerprint hash(k);
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

    int core_number = 1;//Core::get_cores(); // TODO doesn't work if the number of cores doesn't match in comp and decomp

    int max_block_size = 1024 * 1024 * 10; // TODO needs max block size as the block size is read after the allocation of these buffers

    RabinFingerprint hash(k);

    std::vector<Core> cores;
    for (int i = 0; i < core_number; i++) {
        cores.emplace_back(k, max_block_size, max_block_size * 4 + 1024, hash);
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

    RabinFingerprint hash(k);
    MTFHashTableStream<uint64_t> mtf(k, 1024 * 1024, hash); // 1 MB block size
    mtf.decode(in_file, out_file);

    in_file.close();
    out_file.close();

    return 0;
}
