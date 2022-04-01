
#ifndef MTF_TABULATIONHASH_H
#define MTF_TABULATIONHASH_H

#include <cstdint>
#include <vector>
#include <cmath>
#include <random>
#include "Hash.h"

class TabulationHash : public Hash {
private:
    uint64_t i = 0;
    uint64_t kmer_hash = 0;

    // Number of bits in the key to be hashed
    uint64_t p = 0;
    // Number of bits to output as hash
    uint64_t q = 0;
    // Block size (r <= p)
    uint64_t r = 8;
    // Number of blocks to represent a key
    uint64_t t = 0;
    // Table of random numbers
    uint64_t *T;

    int len;

public:
    explicit TabulationHash(int k, int size) : Hash(k, size), p(k * 8), q((uint64_t) log2(size)) {
        t = ceil((double) p / (double) r);
        len = (int) pow(2, (double) r);
        T = new uint64_t[len * t];

        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis;

        for (int j1 = 0; j1 < t; j1++) {
            for (int j2 = 0; j2 < len; j2++) {
                T[j1 * len + j2] = dis(gen);
            }
        }
    }

    TabulationHash(const TabulationHash& hash) = default;

    ~TabulationHash() override {
        delete[] T;
    }

    void init(const std::vector<uint8_t> &start) override {
        hash = 0;
        // First k-mer
        for (int j = 0; j < k; j++) {
            uint8_t c = start[j];
            kmer[j] = c;

            kmer_hash = (kmer_hash << 8) | c;
        }
        for (int j = 0; j < t; j++) {
            hash ^= T[j * len + (kmer_hash >> (r * j)) & 0xFF]; // TODO instead of FF should be r ones
        }
        i = 0;
    }

    void resize(uint64_t size) override {
        this->size = size;
        q = (uint64_t) log2((double) size);
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

        hash = 0;
        for (int j = 0; j < t; j++) {
            hash ^= T[j * len + (kmer_hash >> (r * j)) & 0xFF];
        }
    }
};

#endif //MTF_TABULATIONHASH_H
