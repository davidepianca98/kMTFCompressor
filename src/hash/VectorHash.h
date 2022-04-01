

#ifndef MTF_VECTORHASH_H
#define MTF_VECTORHASH_H

#include <vector>
#include "Hash.h"

class VectorHash : public Hash {

    uint64_t sum = 0;
    uint64_t i = 0;

    void compute() {
        // Sum is the dot product with the unit diagonal vector (1, ..., 1)
        // k is the same as the dot product of the unit vector with itself
        hash = (uint64_t) (((double) sum / (double) (k * 255)) * size);
    }

public:
    explicit VectorHash(int k, int size) : Hash(k, size) {}

    void init(const std::vector<uint8_t> &start) override {
        sum = 0;
        i = 0;
        for (int j = 0; j < k; j++) {
            kmer[j] = start[j];
            sum += kmer[j];
        }
        compute();
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
        sum -= old;
        sum += c;
        compute();
    }
};


#endif //MTF_VECTORHASH_H
