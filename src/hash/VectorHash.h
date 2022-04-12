

#ifndef MTF_VECTORHASH_H
#define MTF_VECTORHASH_H

#include "Hash.h"

class VectorHash : public Hash {

    uint64_t sum = 0;

    static constexpr uint64_t max_size = UINT64_MAX - 1;

public:
    VectorHash(int k, int size);

    uint8_t update(uint8_t c) override;
};


#endif //MTF_VECTORHASH_H
