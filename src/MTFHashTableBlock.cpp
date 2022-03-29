
#include <boost/multiprecision/cpp_int.hpp>
#include "MTFHashTableBlock.h"
#include "RabinKarp.h"

template <typename T>
MTFHashTableBlock<T>::MTFHashTableBlock(int block_size, Hash& hash) : MTFHashTable<T>(block_size, hash) {}

template <typename T>
void MTFHashTableBlock<T>::encode(const uint8_t *block, long size, uint32_t *out_block) {
    std::vector<uint8_t> start(this->hash_function.get_window_size());
    int i;
    for (i = 0; i < start.size(); i++) {
        start[i] = block[i];
        out_block[i] = (uint32_t) block[i] + this->byte_size();
    }

    this->hash_function.init(start);

    for (; i < size; i++) {
        out_block[i] = this->mtfEncode(block[i]);
        this->double_table();
    }
}

template <typename T>
void MTFHashTableBlock<T>::decode(const uint32_t *block, long size, uint8_t *out_block) {
    std::vector<uint8_t> start(this->hash_function.get_window_size());
    int i;
    for (i = 0; i < start.size(); i++) {
        start[i] = (uint8_t) (block[i] - this->byte_size());
        out_block[i] = start[i];
    }

    this->hash_function.init(start);

    for (; i < size; i++) {
        out_block[i] = this->mtfDecode(block[i]);
        this->double_table();
    }
}

template class MTFHashTableBlock<uint16_t>;
template class MTFHashTableBlock<uint32_t>;
template class MTFHashTableBlock<uint64_t>;
template class MTFHashTableBlock<boost::multiprecision::uint128_t>;
template class MTFHashTableBlock<boost::multiprecision::uint256_t>;
template class MTFHashTableBlock<boost::multiprecision::uint512_t>;
template class MTFHashTableBlock<boost::multiprecision::uint1024_t>;
