
#ifndef MTF_MTFHASHTABLEBLOCK_H
#define MTF_MTFHASHTABLEBLOCK_H

#include "MTFHashTable.h"

class MTFHashTableBlock : public MTFHashTable {

public:
    MTFHashTableBlock(int k, int block_size);

    void encode(const uint8_t *block, long size, uint32_t *out_block);

    void decode(const uint32_t *block, long size, uint8_t *out_block);
};


#endif //MTF_MTFHASHTABLEBLOCK_H
