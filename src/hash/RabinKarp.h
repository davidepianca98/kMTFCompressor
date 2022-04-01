
#ifndef MTF_RABINKARP_H
#define MTF_RABINKARP_H


#include <cstdint>
#include <vector>
#include "Hash.h"

class RabinKarp : public Hash {
private:
    // Prime number slightly bigger than the alphabet size
    static constexpr uint32_t x = 257;
    // Multiplier to shift left
    uint64_t xk = 1;

    uint64_t i = 0;

    static constexpr uint64_t P10M = 10000019;
    static constexpr uint64_t P100M = 100000007;

    static constexpr uint64_t q = P100M;

public:
    explicit RabinKarp(int k, int size) : Hash(k, size) {}

    RabinKarp(const RabinKarp& hash) : i(hash.i), xk(hash.xk), Hash(hash) {}

    void init(const std::vector<uint8_t> &start) override {
        hash = 0;
        // First k-mer
        for (int j = 0; j < k; j++) {
            uint8_t c = start[j];
            kmer[j] = c;

            // Multiply the hash by the multiplier to "shift left" and add the new character (push right)
            hash = (hash * x) + c;
        }
        hash %= q;
        i = 0;

        // Build the multiplier (power) for the leftmost character, needed to remove it when updating
        xk = 1;
        for (int j = 0; j < k - 1; j++) {
            xk = (xk * x) % q;
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
        // Shift left by the multiplier
        // Push the new character to the right
        hash = (((hash - (xk * old)) * x) + c) % q;
    }
};


#endif //MTF_RABINKARP_H
