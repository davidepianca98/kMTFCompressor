
#ifndef MTF_IDENTITY_H
#define MTF_IDENTITY_H

#include <cstdint>
#include <vector>
#include "Hash.h"

class Identity : public Hash {
private:
    uint64_t i = 0;

public:
    explicit Identity(int k, int size) : Hash(k, k, size) {}

    void init(const std::vector<uint8_t> &start) override {
        hash = 0;

        // First k-mer
        for (i = 0; i < k; i++) {
            kmer[i] = start[i];
            hash += kmer[i] << (i * 8);
        }

        hash = hash % size;
    }

    void resize(uint64_t size) override {
        this->size = size;
    }

    void update(uint8_t c) override {
        kmer[i] = c;
        i = (i + 1) % k;

        hash <<= 8;
        hash += c; // TODO assumes k <= 8
        hash %= size;
    }

    [[nodiscard]] uint64_t get_hash() const override {
        // Resize for the table size
        return Hash::get_hash() % size;
    }

    [[nodiscard]] uint64_t get_hash_full() const {
        return Hash::get_hash();
    }
};

#endif //MTF_IDENTITY_H
