
#ifndef MTF_JUMPCONSISTENT_H
#define MTF_JUMPCONSISTENT_H

#include <cstdint>
#include <vector>
#include "Hash.h"

class JumpConsistent : public Hash {
private:
    uint64_t i = 0;
    uint64_t key = 0;

    constexpr static double B = double(1ll << 31);

    uint64_t jump_consistent_hash(uint64_t key) {
        uint64_t b = -1, j = 0;
        while (j < size) {
            b = j;
            key = key * 2862933555777941757ull + 1;
            j = (b + 1) * (uint64_t) (B / double((key >> 33) + 1));
        }
        return b;
    }

public:
    explicit JumpConsistent(int k, int size) : Hash(k, k, size) {}

    void init(const std::vector<uint8_t> &start) override {
        hash = 0;
        key = 0;

        // First k-mer
        for (int j = 0; j < k; j++) {
            uint8_t c = start[j];
            kmer[j] = c;

            key = (key << 8) | c;
        }
        hash = jump_consistent_hash(key);
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

        key -= old << (k * 8); // TODO should be k-1, but this works better
        key = (key << 8) | c;

        hash = jump_consistent_hash(key);
    }

    [[nodiscard]] uint64_t get_hash() const override {
        // Resize for the table size
        return Hash::get_hash() % size;
    }

    [[nodiscard]] uint64_t get_hash_full() const {
        return Hash::get_hash();
    }
};

#endif //MTF_JUMPCONSISTENT_H
