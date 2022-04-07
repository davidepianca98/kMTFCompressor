
#ifndef MTF_RABINKARP_H
#define MTF_RABINKARP_H


#include <cstdint>
#include <vector>
#include "Hash.h"

class RabinKarp : public Hash {
private:
    // Prime number slightly bigger than the alphabet size
    static constexpr uint64_t x = 7483268523; // TODO generate randomly from seed
    // Multiplier to shift left (depends on k)
    uint64_t xk = 1;

    uint64_t i = 0;

    static constexpr uint64_t M61 = 2305843009213693951;

public:
    explicit RabinKarp(int k, int size) : Hash(k, size) {} // TODO pass seed on all hashes

    RabinKarp(const RabinKarp& hash) : i(hash.i), xk(hash.xk), Hash(hash) {}

    static inline uint64_t fast_modulo(uint64_t val) {
        uint64_t res = (val & M61) + (val >> 61);
        return (res >= M61) ? res - M61 : res;
    }

    void init(const std::vector<uint8_t> &start) override {
        hash = 0;
        // First k-mer
        for (int j = 0; j < k; j++) {
            uint8_t c = start[j];
            kmer[j] = c;

            // Multiply the hash by the multiplier to "shift left"
            hash = (hash * x) % M61;
            // Add the new character (push right)
            hash = (hash + c) % M61;
        }

        i = 0;

        // Build the multiplier (power) for the leftmost character, needed to remove it when updating
        xk = 1;
        for (int j = 0; j < k - 1; j++) {
            xk = (xk * x) % M61;
        }
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

        // Remove the leftmost character using the multiplier
        hash = (hash - ((xk * old) % M61)) % M61;
        // Shift left by the multiplier
        hash = (hash * x) % M61;
        // Push the new character to the right
        hash = (hash + c) % M61;
    }
};


#endif //MTF_RABINKARP_H
