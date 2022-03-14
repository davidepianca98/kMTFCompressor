
#ifndef MTF_CONCATENATEDHASH_H
#define MTF_CONCATENATEDHASH_H

#include <cmath>
#include <vector>
#include "Hash.h"
#include "RabinKarp.h"
#include "Fnv1a.h"

template <typename HASH1, typename HASH2>
class ConcatenatedHash : public Hash {
    HASH1 hash_kmer;
    HASH2 hash_window;
    int sh;
    int first_size_log;

public:
    ConcatenatedHash(int k, int window_size, int size) : Hash(k, window_size, size), hash_kmer(k, size),
                                    hash_window(window_size, size), first_size_log((int) log2(size) - 2),
                                    sh((int) log2((int) log2(size) - ((int) log2(size) - 2))) {};

    void init(const std::vector<uint8_t> &start) override {
        hash_kmer.init(start);
        hash_window.init(start);

        for (int i = k; i < start.size(); i++) {
            hash_kmer.update(start[i]);
        }

        hash = (hash_kmer.get_hash_full() << sh) | (hash_window.get_hash_full() & (~0ul >> (64 - sh)));
        hash = hash % size;
    }

    void resize(uint64_t size) override {
        this->size = size;
        sh = (int) log2((int) log2(size) - first_size_log);
    }

    void update(uint8_t c) override {
        hash_kmer.update(c);
        hash_window.update(c);

        hash = (hash_kmer.get_hash_full() << sh) | (hash_window.get_hash_full() & (~0ul >> (64 - sh)));
        hash = hash % size;
    }
};

#endif //MTF_CONCATENATEDHASH_H
