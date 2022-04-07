
#ifndef MTF_LINEARHASH_H
#define MTF_LINEARHASH_H

#include <cstdint>
#include <vector>
#include "Hash.h"

class LinearHash : public Hash {
private:
    uint64_t i = 0;
    uint64_t kmer_hash = 0;

    uint64_t a;
    uint64_t b;

public:
    LinearHash(int k, uint64_t seed) : Hash(k, seed) {
        a = dis(gen);
        b = dis(gen);
        hash = 0;
        kmer_hash = 0;
    }

    void update(uint8_t c) override {
        // Update k-mer
        uint8_t old = kmer[i];
        kmer[i] = c;

        // Faster than wrapping with modulo
        i++;
        if (i >= k) {
            i = 0;
        }

        kmer_hash -= old << ((k - 1) * 8);
        kmer_hash = (kmer_hash << 8) | c;

        hash = (a * kmer_hash + b) % M61;
    }
};

#endif //MTF_LINEARHASH_H
