
#ifndef MTF_MTFBLOCKWORKER_H
#define MTF_MTFBLOCKWORKER_H


#include <vector>
#include <cstdint>
#include <future>
#include "encoders/FastPForEncoder.h"
#include "MTFHashTable.h"
#include "MTFHashTableBlock.h"
#include "stream/obufbitstream.h"
#include "encoders/AdaptiveHuffman.h"
#include "stream/ibufbitstream.h"

template <typename HASH>
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

        MTFHashTableBlock<uint64_t> mtf(k, size, std::ref(hash));
        mtf.encode(in, size, out_block1);

        obufbitstream buf(final_block, size); // TODO seems like it outputs many zeros in the end
        AdaptiveHuffman ah(8);
        for (int i = 0; i < size; i++) {
            ah.encode(out_block1[i], buf);
        }
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
        AdaptiveHuffman ah(8);
        uint32_t decompressed_size = 0;
        while ((out_block1[decompressed_size] = ah.decode(buf)) != -1) {
            decompressed_size++;
        }

        MTFHashTableBlock<uint64_t> mtf(k, size, std::ref(hash));
        mtf.decode(out_block1, (long) decompressed_size, final_block);

        delete[] out_block1;
        return decompressed_size;
    }

public:

    MTFBlockWorker(int k, int hash_size, int in_block_size, int out_block_size) : k(k), hash_size(hash_size), in_block(in_block_size), out_block(out_block_size), hash(k, hash_size) {}

    void startCompression(long size) {
        if (!valid && size > 0) {
            hash = HASH(k, hash_size);
            future = std::async(std::launch::async, &MTFBlockWorker<HASH>::compressBlock, this, in_block.data(), size, out_block.data());
            valid = true;
        }
    }

    void startDecompression(long size) {
        if (!valid && size > 0) {
            hash = HASH(k, hash_size);
            future = std::async(std::launch::async, &MTFBlockWorker<HASH>::decompressBlock, this, in_block.data(), size, out_block.data());
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
