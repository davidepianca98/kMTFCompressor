
#ifndef MTF_MTFHASHTABLEBLOCK_H
#define MTF_MTFHASHTABLEBLOCK_H

#include "MTFHashTable.h"

template <typename T>
class MTFHashTableBlock : public MTFHashTable<T> {

public:
    MTFHashTableBlock(int k, int block_size, Hash& hash);

    void encode(const uint8_t *block, long size, uint32_t *out_block);

    void decode(const uint32_t *block, long size, uint8_t *out_block);
};


#endif //MTF_MTFHASHTABLEBLOCK_H
