

#ifndef MTF_MINIMISERHASH_H
#define MTF_MINIMISERHASH_H


#include <vector>
#include "Hash.h"
#include "RabinFingerprint.h"

class MinimiserHash : public Hash {
    int k;
    int window_size;
    int last_update;

    RabinFingerprint rf;

    // Rolling k-mer
    std::vector<uint8_t> kmer;
    std::vector<uint32_t> window_hashes;

    int i = 0;

    static constexpr int size = 10000;

    uint32_t fnv1a();

public:
    MinimiserHash(int k, int window_size, const std::vector<uint8_t> &start);

    void update(uint8_t c) override;
};


#endif //MTF_MINIMISERHASH_H
