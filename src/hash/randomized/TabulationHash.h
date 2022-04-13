
#ifndef MTF_TABULATIONHASH_H
#define MTF_TABULATIONHASH_H

#include "Hash.h"

class TabulationHash : public Hash {
private:
    uint64_t kmer_hash = 0;
    uint8_t *kmer_hash_p = reinterpret_cast<uint8_t *>(&kmer_hash);
    int last_index;

    static constexpr int MAX_KMER = 8;
    static constexpr int LEN = 256;

    // Table of random numbers
    uint64_t T[MAX_KMER][LEN] = { { 0 } };

    uint64_t get_random_uint64();

public:
    TabulationHash(int k, uint64_t seed);

    uint8_t update(uint8_t c) override;
};

#endif //MTF_TABULATIONHASH_H
