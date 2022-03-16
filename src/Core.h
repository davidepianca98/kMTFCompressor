
#ifndef MTF_CORE_H
#define MTF_CORE_H


#include <vector>
#include <cstdint>
#include <future>
#include "MTFHashTable.h"
#include "MTFHashTableBlock.h"

class Core {

    int k;
    bool valid = false;
    std::future<uint32_t> future;

    uint32_t compressBlock(const uint8_t *in_block, long size, uint8_t *final_block);

    uint32_t decompressBlock(const uint8_t *in_block, long size, uint8_t *final_block);

public:
    std::vector<uint8_t> block;
    std::vector<uint8_t> out_block;

    Core(int k, int in_block_size, int out_block_size);

    static void compress_final(const uint32_t *block, size_t size, uint32_t *out_block, size_t& compressed_size);

    static void decompress_final(const uint32_t *data, size_t size, uint32_t *out_block, size_t& decompressed_size);

    void startCompression(long size);

    void startDecompression(long size);

    uint32_t get();

    static int get_cores();
};


#endif //MTF_CORE_H
