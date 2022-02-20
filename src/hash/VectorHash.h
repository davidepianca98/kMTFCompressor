

#ifndef MTF_VECTORHASH_H
#define MTF_VECTORHASH_H


#include "Hash.h"

class VectorHash : public Hash {
    uint64_t b;

    // Rolling k-mer
    std::vector<uint8_t> kmer;
    std::vector<uint8_t> v;

    static uint64_t dot(std::vector<uint8_t> u, std::vector<uint8_t> v);
    void compute();

public:
    static constexpr int size = 1000;

    VectorHash(int k, const std::vector<uint8_t> &start);

    void update(uint8_t c) override;
};


#endif //MTF_VECTORHASH_H
