
#include "MTFHashTableBlock.h"
#include "RabinFingerprint.h"

MTFHashTableBlock::MTFHashTableBlock(int k, int block_size) : MTFHashTable(k, block_size) {}

void MTFHashTableBlock::encode(const uint8_t *block, long size, uint32_t *out_block) {
    uint8_t c;

    std::vector<uint8_t> start(k);
    int i;
    for (i = 0; i < k; i++) {
        c = block[i];
        start[i] = c;
        out_block[i] = (uint32_t) c + 8;
    }

    // Rolling hash to access the table
    RabinFingerprint hash(k, start);

    while (i < size) {
        c = block[i];
        keep_track(hash.get_hash());

        uint32_t pos = mtfEncode(hash_table[hash.get_hash()], c);
        out_block[i] = pos;
        hash.update(c);
        i++;
    }
}

void MTFHashTableBlock::decode(const uint32_t *block, long size, uint8_t *out_block) {
    uint32_t c;

    std::vector<uint8_t> start(k);
    int i;
    for (i = 0; i < k; i++) {
        c = block[i];
        start[i] = (uint8_t) (c - 8);
        out_block[i] = start[i];
    }

    // Rolling hash to access the table
    RabinFingerprint hash(k, start);

    while (i < size) {
        c = block[i];
        uint8_t ca = mtfDecode(hash_table[hash.get_hash()], c);
        out_block[i] = ca;

        hash.update(ca);
        i++;
    }
}


