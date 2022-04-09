
#include "MTFBlockWorker.h"
#include "stream/ibitstream/ibufbitstream.h"
#include "encoders/RunLength.h"
#include "MTFHashTableBlock.h"
#include "stream/obitstream/obufbitstream.h"
#include "randomized/RabinKarp.h"

template <typename HASH, uint32_t SIZE>
uint32_t MTFBlockWorker<HASH, SIZE>::compressBlock(const uint8_t *in, int size, uint8_t *final_block) {
    auto *out_block1 = new uint32_t[size];

    MTFHashTableBlock<HASH, SIZE> mtf(size, max_memory_usage, k, seed);
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

template <typename HASH, uint32_t SIZE>
uint32_t MTFBlockWorker<HASH, SIZE>::decompressBlock(uint8_t *in, int size, uint8_t *final_block) {
    auto *out_block1 = new uint32_t[in_block.size()];

    ibufbitstream buf(in, size);
    RunLength rle(256 + SIZE + 1);
    uint32_t decompressed_size = rle.decode_array(buf, out_block1, in_block.size(), 256 + SIZE);

    MTFHashTableBlock<HASH, SIZE> mtf(decompressed_size, max_memory_usage, k, seed);
    mtf.decode(out_block1, (long) decompressed_size, final_block);

    delete[] out_block1;
    return decompressed_size;
}

template <typename HASH, uint32_t SIZE>
MTFBlockWorker<HASH, SIZE>::MTFBlockWorker(int k, uint64_t seed, int in_block_size, int out_block_size, uint64_t max_memory_usage) : k(k), seed(seed), max_memory_usage(max_memory_usage), in_block(in_block_size), out_block(out_block_size) {}

template <typename HASH, uint32_t SIZE>
void MTFBlockWorker<HASH, SIZE>::startCompression(long size) {
    if (!valid && size > 0) {
        future = std::async(std::launch::async, &MTFBlockWorker<HASH, SIZE>::compressBlock, this, in_block.data(), size, out_block.data());
        valid = true;
    }
}

template <typename HASH, uint32_t SIZE>
void MTFBlockWorker<HASH, SIZE>::startDecompression(long size) {
    if (!valid && size > 0) {
        future = std::async(std::launch::async, &MTFBlockWorker<HASH, SIZE>::decompressBlock, this, in_block.data(), size, out_block.data());
        valid = true;
    }
}

template <typename HASH, uint32_t SIZE>
uint32_t MTFBlockWorker<HASH, SIZE>::get() {
    if (valid) {
        uint32_t res = future.get();
        valid = false;
        return res;
    }
    return 0;
}

template <typename HASH, uint32_t SIZE>
uint8_t *MTFBlockWorker<HASH, SIZE>::get_in_block() {
    return in_block.data();
}

template <typename HASH, uint32_t SIZE>
int MTFBlockWorker<HASH, SIZE>::get_in_block_size() {
    return in_block.size();
}

template <typename HASH, uint32_t SIZE>
uint8_t const *MTFBlockWorker<HASH, SIZE>::get_out_block() {
    return out_block.data();
}

template class MTFBlockWorker<RabinKarp, 8>;
