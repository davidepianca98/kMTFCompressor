
#ifndef MTF_LINEARHASH_H
#define MTF_LINEARHASH_H

#include <cstdint>
#include <vector>
#include "Hash.h"

class LinearHash : public Hash {
private:
    uint64_t i = 0;
    uint64_t kmer_hash = 0;

    static constexpr uint64_t a = 12828009;
    static constexpr uint64_t b = 77602751;
    static constexpr uint64_t q = 100000007;

public:
    explicit LinearHash(int k, int size) : Hash(k, size) {}

    LinearHash(const LinearHash& hash) = default;

    void init(const std::vector<uint8_t> &start) override {
        hash = 0;
        kmer_hash = 0;
        // First k-mer
        for (int j = 0; j < k; j++) {
            uint8_t c = start[j];
            kmer[j] = c;

            kmer_hash = (kmer_hash << 8) | c;
        }
        hash = (a * kmer_hash + b) % q;
        i = 0;
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

        hash = (a * kmer_hash + b) % q;
    }
};

#endif //MTF_LINEARHASH_H
