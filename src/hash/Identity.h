
#ifndef MTF_IDENTITY_H
#define MTF_IDENTITY_H

#include <cstdint>
#include <vector>
#include "Hash.h"

class Identity : public Hash {
private:
    uint64_t i = 0;
    uint64_t sh = 0;

public:
    Identity(int k, uint64_t seed) : Hash(k, seed) {
        assert(k <= 8);
        sh = (~0ul >> (64 - (8 * k)));
        hash = 0;
    }

    Identity(const Identity& hash) = default;

    void init(const std::vector<uint8_t> &start) override {
        hash = 0;

        // First k-mer
        for (i = 0; i < k; i++) {
            kmer[i] = start[i];
            hash = (hash << 8) | kmer[i];
        }
    }

    void update(uint8_t c) override {
        kmer[i] = c;
        i = (i + 1) % k;

        hash = (hash << 8) | c;
        hash &= sh;
    }
};

#endif //MTF_IDENTITY_H
