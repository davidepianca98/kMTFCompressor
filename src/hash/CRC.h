
#ifndef MTF_CRC_H
#define MTF_CRC_H

#include <cstdint>
#include <vector>
#include "Hash.h"

class CRC : public Hash {
private:
    uint64_t i = 0;
    uint64_t mask1;
    uint64_t maskn;

    static constexpr int WORD_SIZE = 32;

    uint64_t fastleftshiftn(uint64_t x)  {
        return ((x & maskn) << WORD_SIZE) | (x >> (WORD_SIZE - k));
    }

    [[nodiscard]] uint64_t fastleftshift1(uint64_t x) const  {
        return ((x & mask1) << 1) | (x >> (WORD_SIZE - 1));
    }

public:
    explicit CRC(int k, int size) : Hash(k, k, size) {}

    void init(const std::vector<uint8_t> &start) override {
        hash = 0;
        mask1 = 1ul << (WORD_SIZE - 1 - 1);
        mask1 ^= mask1 - 1;

        maskn = 1ul << (WORD_SIZE - k - 1);
        maskn ^= maskn - 1;

        // First k-mer
        for (int j = 0; j < k; j++) {
            uint8_t c = start[j];
            kmer[j] = c;

            hash = fastleftshift1(hash);
            hash ^= c;
        }
        i = 0;
    }

    void resize(uint64_t size) override {
        this->size = size;
    }

    void update(uint8_t c) override {
        // Update k-mer
        uint8_t old = kmer[i];
        kmer[i] = c;
        i = (i + 1) % k;

        hash = fastleftshift1(hash) ^ fastleftshiftn(old) ^ c;
    }

    [[nodiscard]] uint64_t get_hash() const override {
        // Resize for the table size
        return Hash::get_hash() % size;
    }

    [[nodiscard]] uint64_t get_hash_full() const {
        return Hash::get_hash();
    }
};

#endif //MTF_CRC_H