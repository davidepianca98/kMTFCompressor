
#include "MTFHashTableBlock.h"
#include "randomized/RabinKarp.h"
#include "randomized/LinearHash.h"
#include "randomized/MinimiserHash.h"

template <typename HASH, uint32_t SIZE>
MTFHashTableBlock<HASH, SIZE>::MTFHashTableBlock(int block_size, uint64_t max_memory_usage, int k, uint64_t seed) : MTFHashTable<HASH, SIZE>(block_size, max_memory_usage, k, seed) {}

template <typename HASH, uint32_t SIZE>
void MTFHashTableBlock<HASH, SIZE>::encode(const uint8_t *block, long size, uint32_t *out_block) {
    for (int i = 0; i < size; i++) {
        out_block[i] = MTFHashTable<HASH, SIZE>::mtf_encode(block[i]);
    }
}

template <typename HASH, uint32_t SIZE>
void MTFHashTableBlock<HASH, SIZE>::decode(const uint32_t *block, long size, uint8_t *out_block) {
    for (int i = 0; i < size; i++) {
        out_block[i] = MTFHashTable<HASH, SIZE>::mtf_decode(block[i]);
    }
}

template class MTFHashTableBlock<RabinKarp, 2>;
template class MTFHashTableBlock<RabinKarp, 4>;
template class MTFHashTableBlock<RabinKarp, 8>;
template class MTFHashTableBlock<RabinKarp, 16>;
template class MTFHashTableBlock<RabinKarp, 32>;
template class MTFHashTableBlock<RabinKarp, 64>;
template class MTFHashTableBlock<RabinKarp, 128>;
template class MTFHashTableBlock<RabinKarp, 256>;

template class MTFHashTableBlock<LinearHash, 8>;
template class MTFHashTableBlock<MinimiserHash<RabinKarp, LinearHash, RabinKarp>, 8>;
