
#include <iostream>
#include <fastpfor/codecs.h>
#include <fastpfor/codecfactory.h>
#include <thread>
#include <future>
#include "MTFHash.h"
#include "MTFHashTable.h"
#include "Core.h"

std::string codec_name = "simdoptpfor"; // simdoptpfor, simdfastpfor256

void compress_final(const uint32_t *block, size_t size, uint32_t *out_block, size_t& compressed_size) {
    using namespace FastPForLib;
    IntegerCODEC *codec = new CompositeCodec<SIMDOPTPFor<4, Simple16<false>>, VariableByte>();

    codec->encodeArray(block, size, out_block, compressed_size);
    compressed_size *= 4;
    delete codec;
}

void enlarge(const uint8_t *block, long size, uint8_t *out_block) {
    for (int i = 0; i < size; i++) {
        out_block[i * 4] = block[i];
        out_block[(i * 4) + 1] = 0;
        out_block[(i * 4) + 2] = 0;
        out_block[(i * 4) + 3] = 0;
    }
}

void reduce(const uint8_t *block, long size, uint8_t *out_block) {
    for (int i = 0; i < size; i += 4) {
        out_block[i / 4] = block[i];
    }
}

uint32_t compressBlock(const uint8_t *block, long size, int k, uint8_t *final_block) {
    uint32_t out_block[size];

    MTFHashTable mtf(k);
    mtf.encode(block, size, out_block);

    mtf.print_stats();

    for (int i = 0; i < size; i++) {
        //std::cout << out_block[i] << " ";
    }

    size_t compressed_size = size * 4 + 1024; // TODO probably pass as parameter

    compress_final(out_block, size, reinterpret_cast<uint32_t *>(final_block), compressed_size);
    return (uint32_t) compressed_size;
}

int get_cores() {
    unsigned int processor_count = std::thread::hardware_concurrency();
    if (processor_count == 0) {
        processor_count = 1;
    }
    return (int) processor_count;
}

int MTFHash::compress(const std::string& path, const std::string& out_path, int k) {
    std::ifstream in_file(path, std::ios::binary);
    if (in_file.fail()) {
        return 1;
    }
    std::ofstream out_file(out_path, std::ios::binary);

    int core_number = get_cores();

    int block_size = 1024 * 1024; // 1 MB block size

    std::vector<Core> cores;
    for (int i = 0; i < core_number; i++) {
        cores.emplace_back(block_size, block_size * 4 + 1024);
    }

    while (in_file.good()) {
        for (Core& core: cores) {
            in_file.read(reinterpret_cast<char *>(core.block.data()), core.block.size());
            long read_bytes = in_file.gcount();

            // TODO spawn threads that compress, maybe spawn with different k, and save in block header the best k and the compressed data
            core.start(compressBlock, read_bytes, k);
        }

        for (Core& core: cores) {
            uint32_t compressed_block_size = core.get();
            if (compressed_block_size > 0) {
                out_file.write(reinterpret_cast<const char *>(&compressed_block_size), 4);
                out_file.write(reinterpret_cast<const char *>(&k), 4);
                out_file.write(reinterpret_cast<const char *>(core.out_block.data()), compressed_block_size);
            }
        }
    }

    in_file.close();
    out_file.close();

    return 0;
}

void decompress_final(const uint32_t *data, size_t size, uint32_t *out_block, size_t& decompressed_size) {
    using namespace FastPForLib;
    IntegerCODEC *codec = new CompositeCodec<SIMDOPTPFor<4, Simple16<false>>, VariableByte>();

    codec->decodeArray(data, size, out_block, decompressed_size);
    delete codec;
}

uint32_t decompressBlock(const uint8_t *block, long size, int k, uint8_t *final_block) {
    size_t decompressed_size = ((uint32_t *) block)[0] * 4 * 32 + 1024; // TODO probably move in final
    uint32_t out_block[decompressed_size];
    decompress_final(reinterpret_cast<const uint32_t *>(block), size / 4, out_block, decompressed_size);

    MTFHashTable mtf(k);
    mtf.decode(out_block, decompressed_size, final_block);

    return (uint32_t) decompressed_size;
}

int MTFHash::decompress(const std::string &path, const std::string &out_path) {
    std::ifstream in_file(path, std::ios::binary);
    if (in_file.fail()) {
        return 1;
    }
    std::ofstream out_file(out_path, std::ios::binary);

    int core_number = get_cores();

    int max_block_size = 1024 * 1024 * 10; // TODO needs max block size as the block size is read after the allocation of these buffers

    std::vector<Core> cores;
    for (int i = 0; i < core_number; i++) {
        cores.emplace_back(max_block_size, max_block_size * 4 + 1024);
    }

    while (in_file.good()) {
        for (Core& core: cores) {
            uint32_t block_size;
            in_file.read(reinterpret_cast<char *>(&block_size), 4);

            int k;
            in_file.read(reinterpret_cast<char *>(&k), 4);

            in_file.read(reinterpret_cast<char *>(core.block.data()), block_size);
            long read_bytes = in_file.gcount(); // TODO check read_byte is equal to block_size

            core.start(decompressBlock, read_bytes, k);
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
