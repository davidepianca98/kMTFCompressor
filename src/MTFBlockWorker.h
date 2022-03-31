
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
    int hash_size;
    bool valid = false;
    std::future<uint32_t> future;

    HASH hash;

    std::vector<uint8_t> in_block;
    std::vector<uint8_t> out_block;

    uint32_t compressBlock(const uint8_t *in, int size, uint8_t *final_block) {
        auto *out_block1 = new uint32_t[size];

        MTFHashTableBlock<T> mtf(size, std::ref(hash));
        mtf.encode(in, size, out_block1);

        obufbitstream buf(final_block, out_block.size());
        AdaptiveHuffman ah(256 + byte_size() + 1);
        for (int i = 0; i < size; i++) {
            ah.encode(out_block1[i], buf);
        }
        ah.encode(256 + byte_size(), buf); // EOF
        buf.flush_remaining();
        uint32_t compressed_size = buf.size();

        //uint32_t compressed_size = FastPForEncoder::compress(out_block1, size, reinterpret_cast<uint32_t *>(final_block));

        delete[] out_block1;
        return compressed_size;
    }

    uint32_t decompressBlock(uint8_t *in, int size, uint8_t *final_block) {
        auto *out_block1 = new uint32_t[in_block.size()];

        //uint32_t decompressed_size = FastPForEncoder::decompress(reinterpret_cast<const uint32_t *>(in), size / 4, out_block1);

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

        MTFHashTableBlock<T> mtf(decompressed_size, std::ref(hash));
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

    MTFBlockWorker(int k, int hash_size, int in_block_size, int out_block_size) : k(k), hash_size(hash_size), in_block(in_block_size), out_block(out_block_size), hash(k, hash_size) {}

    void startCompression(long size) {
        if (!valid && size > 0) {
            hash = HASH(k, hash_size);
            future = std::async(std::launch::async, &MTFBlockWorker<HASH, T>::compressBlock, this, in_block.data(), size, out_block.data());
            valid = true;
        }
    }

    void startDecompression(long size) {
        if (!valid && size > 0) {
            hash = HASH(k, hash_size);
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
