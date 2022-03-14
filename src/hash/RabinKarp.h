
#ifndef MTF_RABINKARP_H
#define MTF_RABINKARP_H


#include <cstdint>
#include <vector>
#include "Hash.h"

class RabinKarp : public Hash {
private:
    // Random number
    uint64_t x = 0;
    // Multiplier to shift left
    uint64_t xk = 1;

    uint64_t i = 0;

    //constexpr uint64_t M19 = (uint64_t(1)<<19)-1;
    //constexpr uint64_t M31 = (uint64_t(1)<<31)-1;
    // 1009, 1013, 1019, 10007, 10009, 10037, 100003, 100019, 100043, 1000003, 1000033, 1000037
    static constexpr uint64_t P10M = 10000019;
    static constexpr uint64_t P100M = 100000007;

    static constexpr uint64_t q = P100M;//2147483647; // 2^31 - 1

public:
    explicit RabinKarp(int k, int size = P100M) : Hash(k, k, size) {}

    void init(const std::vector<uint8_t> &start) override {
        // Prime number slightly bigger than the alphabet size
        x = 257;

        // First k-mer
        for (int j = 0; j < k; j++) {
            uint8_t c = start[j];
            kmer[j] = c;

            // Multiply the hash by the multiplier to "shift left"
            hash = (hash * x) % q;
            // Add the new character (push right)
            hash = (hash + c) % q;
        }

        // Build the multiplier (power) for the leftmost character, needed to remove it when updating
        for (int j = 0; j < k - 1; j++) {
            xk = (xk * x) % q;
        }
    }

    void resize(uint64_t size) override {
        this->size = size;
    }

    void update(uint8_t c) override {
        // Update k-mer
        uint8_t old = kmer[i];
        kmer[i] = c;
        i = (i + 1) % k;

        // Remove the leftmost character using the multiplier
        hash = (hash - ((xk * old) % q)) % q;
        // Shift left by the multiplier
        hash = (hash * x) % q;
        // Push the new character to the right
        hash = (hash + c) % q;
    }

    [[nodiscard]] uint64_t get_hash() const override {
        // Resize for the table size
        return Hash::get_hash() % size;
    }

    [[nodiscard]] uint64_t get_hash_full() const {
        return Hash::get_hash();
    }

    void increment_k(uint8_t c) {
        k++;
        std::cout << k << std::endl;
        kmer.resize(k);
        for (uint64_t j = k - 1; j > i; j--) {
            kmer[j] = kmer[j - 1];
        }
        kmer[i] = c;
        hash = (hash * x) % size;
        hash = (hash + c) % size;

        i = (i + 1) % k;

        xk = (xk * x) % size;
    }

    void decrement_k() {
        k--;
        // TODO
        xk = (xk / x) % size;
    }
};


#endif //MTF_RABINKARP_H
