
#ifndef MTF_MTFBLOCKWORKER_H
#define MTF_MTFBLOCKWORKER_H


#include <cstdint>
#include <future>
#include <vector>
#include "MTFBlockWorker.h"
#include "stream/ibitstream/ibufbitstream.h"
#include "encoders/RunLength.h"
#include "MTFHashTable.h"
#include "stream/obitstream/obufbitstream.h"

template <typename HASH, uint32_t SIZE>
class MTFBlockWorker {

    int k;
    uint64_t seed;
    uint64_t max_memory_usage;
    bool valid = false;
    std::future<uint32_t> future;

    std::vector<uint8_t> in_block;
    std::vector<uint8_t> out_block;

    uint32_t compressBlock(const uint8_t *in, int size, uint8_t *final_block) {
        auto *out_block1 = new uint32_t[size];

        MTFHashTable<HASH, SIZE> mtf(size, max_memory_usage, k, seed);
        mtf.encode(in, size, out_block1);

        obufbitstream buf(final_block, out_block.size());
        RunLength rle(256 + SIZE + 1);
        rle.encode_array(out_block1, size, buf);
        rle.encode_end(256 + SIZE, buf); // EOF
        buf.flush_remaining();
        uint32_t compressed_size = buf.size();

        delete[] out_block1;
        return compressed_size;
    }

    uint32_t decompressBlock(uint8_t *in, int size, uint8_t *final_block) {
        auto *out_block1 = new uint32_t[in_block.size()];

        ibufbitstream buf(in, size);
        RunLength rle(256 + SIZE + 1);
        uint32_t decompressed_size = rle.decode_array(buf, out_block1, in_block.size(), 256 + SIZE);

        MTFHashTable<HASH, SIZE> mtf(decompressed_size, max_memory_usage, k, seed);
        mtf.decode(out_block1, (long) decompressed_size, final_block);

        delete[] out_block1;
        return decompressed_size;
    }

public:

    MTFBlockWorker(int k, uint64_t seed, int in_block_size, int out_block_size, uint64_t max_memory_usage) : k(k), seed(seed), max_memory_usage(max_memory_usage), in_block(in_block_size), out_block(out_block_size) {}

    void startCompression(long size) {
        if (!valid && size > 0) {
            future = std::async(std::launch::async, &MTFBlockWorker<HASH, SIZE>::compressBlock, this, in_block.data(), size, out_block.data());
            valid = true;
        }
    }

    void startDecompression(long size) {
        if (!valid && size > 0) {
            future = std::async(std::launch::async, &MTFBlockWorker<HASH, SIZE>::decompressBlock, this, in_block.data(), size, out_block.data());
            valid = true;
        }
    }

    uint32_t get() {
        if (valid) {
            uint32_t res = future.get();
            valid = false;
            return res;
        }
        return 0;
    }

    uint8_t *get_in_block() {
        return in_block.data();
    }

    int get_in_block_size() {
        return in_block.size();
    }

    uint8_t const *get_out_block() {
        return out_block.data();
    }
};


#endif //MTF_MTFBLOCKWORKER_H
