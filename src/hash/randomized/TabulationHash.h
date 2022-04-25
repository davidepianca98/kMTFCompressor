
#ifndef MTF_TABULATIONHASH_H
#define MTF_TABULATIONHASH_H

#include "Hash.h"

class TabulationHash : public Hash {
private:
    static constexpr int MAX_KMER = 8;
    static constexpr int LEN = 256;

    // Table of random numbers
    uint32_t T[MAX_KMER][LEN] = { { 0 } };

    uint64_t get_random_uint64();

    uint32_t get_random_uint32();

public:
    TabulationHash(int k, uint64_t seed);

    uint8_t update(uint8_t c) override;

    uint64_t compute(uint64_t key) override;
};

#endif //MTF_TABULATIONHASH_H
