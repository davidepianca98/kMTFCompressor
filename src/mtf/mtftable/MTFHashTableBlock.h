
#ifndef MTF_MTFHASHTABLEBLOCK_H
#define MTF_MTFHASHTABLEBLOCK_H

#include "MTFHashTable.h"

template <typename HASH, uint32_t SIZE>
class MTFHashTableBlock : public MTFHashTable<HASH, SIZE> {

public:
    MTFHashTableBlock(int block_size, uint64_t max_memory_usage, int k, uint64_t seed);

    void encode(const uint8_t *block, long size, uint32_t *out_block);

    void decode(const uint32_t *block, long size, uint8_t *out_block);
};


#endif //MTF_MTFHASHTABLEBLOCK_H
