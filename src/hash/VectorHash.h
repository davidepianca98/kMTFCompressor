

#ifndef MTF_VECTORHASH_H
#define MTF_VECTORHASH_H


#include "Hash.h"

class VectorHash : public Hash {
    uint64_t b;
    std::vector<uint8_t> v;

    static uint64_t dot(std::vector<uint8_t> u, std::vector<uint8_t> v);
    void compute();

public:
    explicit VectorHash(int k);

    void init(const std::vector<uint8_t> &start) override;

    void resize(uint64_t size) override;

    void update(uint8_t c) override;
};


#endif //MTF_VECTORHASH_H
