
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
    explicit Fnv1a(int k, int size) : Hash(k, size) {}

    void init(const std::vector <uint8_t> &start) override {
        uint32_t hash = BASE;

        // First k-mer
        for (int j = 0; j < k; j++) {
            uint8_t c = start[j];
            kmer[j] = c;

            hash = (hash ^ c) * PRIME;
        }
        this->hash = hash;
    }

    void update(uint8_t c) override {
        // Update k-mer
        uint8_t old = kmer[i];
        kmer[i] = c;
        i = (i + 1) % k;

        uint32_t hash = BASE;
        for (int j = 0; j < k; j++) {
            hash = (hash ^ kmer[(i + j + 1) % k]) * PRIME;
        }
        this->hash = hash;
    }
};

#endif //MTF_FNV1A_H
