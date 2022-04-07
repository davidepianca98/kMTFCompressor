
#ifndef MTF_FNV1A_H
#define MTF_FNV1A_H

#include <vector>
#include "Hash.h"

class Fnv1a : public Hash {
private:
    uint64_t i = 0;

    static constexpr uint64_t BASE = 2166136261;
    //static constexpr uint64_t BASE = 14695981039346656037u;

    static constexpr uint64_t PRIME = 16777619;
    //static constexpr uint64_t PRIME = 1099511628211;

public:
    Fnv1a(int k, uint64_t seed) : Hash(k, seed) {}

    void update(uint8_t c) override {
        // Update k-mer
        kmer[i] = c;
        i = (i + 1) % k;

        uint32_t hash = BASE;
        for (int j = 0; j < k; j++) {
            hash = (hash ^ kmer[(i + j + 1) % k]) * PRIME;
        }
        Hash::hash = hash;
    }
};

#endif //MTF_FNV1A_H
