
#include "MTFBlockWorker.h"
#include "stream/ibitstream/ibufbitstream.h"
#include "encoders/RunLength.h"
#include "MTFHashTableBlock.h"
#include "stream/obitstream/obufbitstream.h"
#include "randomized/RabinKarp.h"

template <typename HASH, typename T>
uint32_t MTFBlockWorker<HASH, T>::compressBlock(const uint8_t *in, int size, uint8_t *final_block) {
    auto *out_block1 = new uint32_t[size];

    MTFHashTableBlock<HASH, T> mtf(size, max_memory_usage, k, seed);
    mtf.encode(in, size, out_block1);

    obufbitstream buf(final_block, out_block.size());
    RunLength rle(256 + byte_size() + 1);
    rle.encode_array(out_block1, size, buf);
    rle.encode(256 + byte_size(), buf); // EOF
    buf.flush_remaining();
    uint32_t compressed_size = buf.size();

    delete[] out_block1;
    return compressed_size;
}

template <typename HASH, typename T>
uint32_t MTFBlockWorker<HASH, T>::decompressBlock(uint8_t *in, int size, uint8_t *final_block) {
    auto *out_block1 = new uint32_t[in_block.size()];

    ibufbitstream buf(in, size);
    RunLength rle(256 + byte_size() + 1);
    uint32_t decompressed_size = rle.decode_array(buf, out_block1, in_block.size(), 256 + byte_size());

    MTFHashTableBlock<HASH, T> mtf(decompressed_size, max_memory_usage, k, seed);
    mtf.decode(out_block1, (long) decompressed_size, final_block);

    delete[] out_block1;
    return decompressed_size;
}

template <typename HASH, typename T>
MTFBlockWorker<HASH, T>::MTFBlockWorker(int k, uint64_t seed, int in_block_size, int out_block_size, uint64_t max_memory_usage) : k(k), seed(seed), in_block(in_block_size), out_block(out_block_size), max_memory_usage(max_memory_usage) {}

template <typename HASH, typename T>
void MTFBlockWorker<HASH, T>::startCompression(long size) {
    if (!valid && size > 0) {
        future = std::async(std::launch::async, &MTFBlockWorker<HASH, T>::compressBlock, this, in_block.data(), size, out_block.data());
        valid = true;
    }
}

template <typename HASH, typename T>
void MTFBlockWorker<HASH, T>::startDecompression(long size) {
    if (!valid && size > 0) {
        future = std::async(std::launch::async, &MTFBlockWorker<HASH, T>::decompressBlock, this, in_block.data(), size, out_block.data());
        valid = true;
    }
}

template <typename HASH, typename T>
uint32_t MTFBlockWorker<HASH, T>::get() {
    if (valid) {
        uint32_t res = future.get();
        valid = false;
        return res;
    }
    return 0;
}

template <typename HASH, typename T>
uint8_t *MTFBlockWorker<HASH, T>::get_in_block() {
    return in_block.data();
}

template <typename HASH, typename T>
int MTFBlockWorker<HASH, T>::get_in_block_size() {
    return in_block.size();
}

template <typename HASH, typename T>
uint8_t const *MTFBlockWorker<HASH, T>::get_out_block() {
    return out_block.data();
}

template class MTFBlockWorker<RabinKarp, uint64_t>;
