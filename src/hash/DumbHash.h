
#ifndef MTF_DUMBHASH_H
#define MTF_DUMBHASH_H


#include <vector>
#include "Hash.h"

class DumbHash : public Hash {

    uint32_t i = 0;

public:
    explicit DumbHash(int k, int size) : Hash(k, k, size) {}

    void init(const std::vector<uint8_t> &start) override {
        // First k-mer
        for (i = 0; i < k; i++) {
            kmer[i] = start[i];
            hash = (hash << 8) | kmer[i];
        }
        i = 0;
    }

    void resize(uint64_t size) override {
        this->size = size;
    }

    void update(uint8_t c) override {
        uint8_t old = kmer[i];
        kmer[i] = c;
        i = (i + 1) % k;

        hash -= old << (k * 8); // TODO should be k-1, but this works better with larger files
        hash = (hash << 8) | c;
    }

    [[nodiscard]] uint64_t get_hash() const override {
        // Resize for the table size
        return Hash::get_hash() % size;
    }

    [[nodiscard]] uint64_t get_hash_full() const {
        return Hash::get_hash();
    }
};


#endif //MTF_DUMBHASH_H
