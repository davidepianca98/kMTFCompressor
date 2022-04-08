
#ifndef MTF_LINEARHASH_H
#define MTF_LINEARHASH_H

#include "Hash.h"

class LinearHash : public Hash {
private:
    uint64_t i = 0;
    uint64_t kmer_hash = 0;

    uint64_t a;
    uint64_t b;

public:
    LinearHash(int k, uint64_t seed);

    void update(uint8_t c) override;
};

#endif //MTF_LINEARHASH_H
