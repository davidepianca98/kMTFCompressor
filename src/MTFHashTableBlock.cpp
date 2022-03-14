
#include <boost/multiprecision/cpp_int.hpp>
#include "MTFHashTableBlock.h"
#include "RabinKarp.h"

template <typename T>
MTFHashTableBlock<T>::MTFHashTableBlock(int k, int block_size, Hash hash) : MTFHashTable<T>(k, block_size, hash) {}

template <typename T>
void MTFHashTableBlock<T>::encode(const uint8_t *block, long size, uint32_t *out_block) {
    std::vector<uint8_t> start(this->k);
    int i;
    for (i = 0; i < this->k; i++) {
        start[i] = block[i];
        out_block[i] = (uint32_t) block[i] + 8;
    }

    this->hash_function.init(start);

    for (; i < size; i++) {
        out_block[i] = this->mtfEncode(block[i]);
    }
}

template <typename T>
void MTFHashTableBlock<T>::decode(const uint32_t *block, long size, uint8_t *out_block) {
    std::vector<uint8_t> start(this->k);
    int i;
    for (i = 0; i < this->k; i++) {
        start[i] = (uint8_t) (block[i] - 8);
        out_block[i] = start[i];
    }

    this->hash_function.init(start);

    for (; i < size; i++) {
        out_block[i] = this->mtfDecode(block[i]);
    }
}

template class MTFHashTableBlock<uint64_t>;
template class MTFHashTableBlock<boost::multiprecision::uint1024_t>;
