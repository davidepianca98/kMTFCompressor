
#include "MTFHashTableBlock.h"
#include "randomized/RabinKarp.h"

template <typename HASH, typename T>
MTFHashTableBlock<HASH, T>::MTFHashTableBlock(int block_size, uint64_t max_memory_usage, int k, uint64_t seed) : MTFHashTable<HASH, T>(block_size, max_memory_usage, k, seed) {}

template <typename HASH, typename T>
void MTFHashTableBlock<HASH, T>::encode(const uint8_t *block, long size, uint32_t *out_block) {
    for (int i = 0; i < size; i++) {
        out_block[i] = MTFHashTable<HASH, T>::mtf_encode(block[i]);
    }
}

template <typename HASH, typename T>
void MTFHashTableBlock<HASH, T>::decode(const uint32_t *block, long size, uint8_t *out_block) {
    for (int i = 0; i < size; i++) {
        out_block[i] = MTFHashTable<HASH, T>::mtf_decode(block[i]);
    }
}

template class MTFHashTableBlock<RabinKarp, uint16_t>;
template class MTFHashTableBlock<RabinKarp, uint32_t>;
template class MTFHashTableBlock<RabinKarp, uint64_t>;
template class MTFHashTableBlock<RabinKarp, boost::multiprecision::uint128_t>;
template class MTFHashTableBlock<RabinKarp, boost::multiprecision::uint256_t>;
template class MTFHashTableBlock<RabinKarp, boost::multiprecision::uint512_t>;
template class MTFHashTableBlock<RabinKarp, boost::multiprecision::uint1024_t>;
