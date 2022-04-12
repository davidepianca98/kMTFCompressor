
#ifndef MTF_TABULATIONHASH_H
#define MTF_TABULATIONHASH_H

#include "Hash.h"

class TabulationHash : public Hash {
private:
    uint64_t kmer_hash = 0;

    // Number of bits in the key to be hashed
    uint64_t p = 0;
    // Block size (r <= p)
    uint64_t r = 8;
    // Number of blocks to represent a key
    uint64_t t = 0;
    // Table of random numbers
    uint64_t *T;

    int len;

public:
    TabulationHash(int k, uint64_t seed);

    ~TabulationHash() override;

    uint8_t update(uint8_t c) override;
};

#endif //MTF_TABULATIONHASH_H
