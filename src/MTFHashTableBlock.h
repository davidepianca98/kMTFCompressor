
#ifndef MTF_MTFHASHTABLEBLOCK_H
#define MTF_MTFHASHTABLEBLOCK_H

#include "MTFHashTable.h"

template <typename HASH, typename T>
class MTFHashTableBlock : public MTFHashTable<HASH, T> {

public:
    MTFHashTableBlock(int block_size, int k, uint64_t seed) : MTFHashTable<HASH, T>(block_size, k, seed) {}

    void encode(const uint8_t *block, long size, uint32_t *out_block) {
        std::vector<uint8_t> start(MTFHashTable<HASH, T>::hash_function.get_length());
        int i;
        for (i = 0; i < start.size(); i++) {
            start[i] = block[i];
            out_block[i] = (uint32_t) block[i] + MTFHashTable<HASH, T>::byte_size();
        }

        MTFHashTable<HASH, T>::hash_function.init(start);

        for (; i < size; i++) {
            out_block[i] = MTFHashTable<HASH, T>::mtfEncode(block[i]);
        }
    }

    void decode(const uint32_t *block, long size, uint8_t *out_block) {
        std::vector<uint8_t> start(MTFHashTable<HASH, T>::hash_function.get_length());
        int i;
        for (i = 0; i < start.size(); i++) {
            start[i] = (uint8_t) (block[i] - MTFHashTable<HASH, T>::byte_size());
            out_block[i] = start[i];
        }

        MTFHashTable<HASH, T>::hash_function.init(start);

        for (; i < size; i++) {
            out_block[i] = MTFHashTable<HASH, T>::mtfDecode(block[i]);
        }
    }
};


#endif //MTF_MTFHASHTABLEBLOCK_H
