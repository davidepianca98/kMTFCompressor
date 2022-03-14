
#ifndef MTF_DUMBHASH_H
#define MTF_DUMBHASH_H


#include <vector>
#include "Hash.h"

class DumbHash : public Hash {

    int i = 0;

public:
    explicit DumbHash(int k, int size) : Hash(k, k, size) {}

    void init(const std::vector<uint8_t> &start) override {
        // First k-mer
        for (i = 0; i < k; i++) {
            kmer[i] = start[i];
            hash += kmer[i] << (i * 8);
        }

        hash = hash % size;
    }

    void resize(uint64_t size) override {
        // TODO
    }

    void update(uint8_t c) override {
        kmer[i] = c;
        i = (i + 1) % k;

        hash >>= 8;
        hash += c << ((k - 1) * 8); // TODO assumes k <= 8
        hash = hash % size;
    }
};


#endif //MTF_DUMBHASH_H
