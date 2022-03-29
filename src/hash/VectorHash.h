

#ifndef MTF_VECTORHASH_H
#define MTF_VECTORHASH_H


#include "Hash.h"

class VectorHash : public Hash {
    uint64_t b;
    std::vector<uint8_t> v;

    static uint64_t dot(std::vector<uint8_t> u, std::vector<uint8_t> v) {
        uint64_t result = 0;
        for (int i = 0; i < u.size(); i++) {
            result += u[i] * v[i];
        }
        return result;
    }

    void compute() {
        uint64_t a = dot(kmer, v);

        hash = (uint64_t) (((double) a / (double) b) * size);
    }

public:
    explicit VectorHash(int k, int size) : Hash(k, k, size), v(k) {
        for (uint8_t& c: v) {
            c = 255;
        }
        b = dot(v, v);
    }

    void init(const std::vector<uint8_t> &start) override {
        for (int i = 0; i < k; i++) {
            kmer[i] = start[i];
        }
        compute();
    }

    void resize(uint64_t size) override {
        this->size = size;
    }

    void update(uint8_t c) override {
        for (int i = 0; i < kmer.size() - 1; i++) {
            kmer[i] = kmer[i + 1];
        }
        kmer[kmer.size() - 1] = c;
        compute();
    }
};


#endif //MTF_VECTORHASH_H
