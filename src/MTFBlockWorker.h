
#ifndef MTF_MTFBLOCKWORKER_H
#define MTF_MTFBLOCKWORKER_H


#include <vector>
#include <cstdint>
#include <future>
#include "MTFHashTable.h"
#include "MTFHashTableBlock.h"
#include "stream/obufbitstream.h"
#include "encoders/AdaptiveHuffman.h"
#include "stream/ibufbitstream.h"

template <typename HASH, typename T>
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

        MTFHashTableBlock<HASH, T> mtf(size, max_memory_usage, k, seed);
        mtf.encode(in, size, out_block1);
        obufbitstream buf(final_block, out_block.size());
        AdaptiveHuffman ah(256 + byte_size() + 1);
        for (int i = 0; i < size; i++) {
            ah.encode(out_block1[i], buf);
        }
        ah.encode(256 + byte_size(), buf); // EOF
        buf.flush_remaining();
        uint32_t compressed_size = buf.size();

        delete[] out_block1;
        return compressed_size;
    }

    uint32_t decompressBlock(uint8_t *in, int size, uint8_t *final_block) {
        auto *out_block1 = new uint32_t[in_block.size()];

        ibufbitstream buf(in, size);
        AdaptiveHuffman ah(256 + byte_size() + 1);
        uint32_t decompressed_size = 0;
        while (true) {
            out_block1[decompressed_size] = ah.decode(buf);
            if (out_block1[decompressed_size] == -1 || out_block1[decompressed_size] == 256 + byte_size()) {
                break;
            }
            decompressed_size++;
        }

        MTFHashTableBlock<HASH, T> mtf(decompressed_size, max_memory_usage, k, seed);
        mtf.decode(out_block1, (long) decompressed_size, final_block);

        delete[] out_block1;
        return decompressed_size;
    }

    constexpr static uint8_t byte_size() noexcept {
        if (std::is_same<T, boost::multiprecision::uint128_t>::value) {
            return 16;
        } else if (std::is_same<T, boost::multiprecision::uint256_t>::value) {
            return 32;
        } else if (std::is_same<T, boost::multiprecision::uint512_t>::value) {
            return 64;
        } else if (std::is_same<T, boost::multiprecision::uint1024_t>::value) {
            return 128;
        } else {
            return sizeof(T);
        }
    }

public:

    MTFBlockWorker(int k, uint64_t seed, int in_block_size, int out_block_size, uint64_t max_memory_usage) : k(k), seed(seed), in_block(in_block_size), out_block(out_block_size), max_memory_usage(max_memory_usage) {}

    void startCompression(long size) {
        if (!valid && size > 0) {
            future = std::async(std::launch::async, &MTFBlockWorker<HASH, T>::compressBlock, this, in_block.data(), size, out_block.data());
            valid = true;
        }
    }

    void startDecompression(long size) {
        if (!valid && size > 0) {
            future = std::async(std::launch::async, &MTFBlockWorker<HASH, T>::decompressBlock, this, in_block.data(), size, out_block.data());
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
