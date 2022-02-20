
#ifndef MTF_DUMBHASH_H
#define MTF_DUMBHASH_H


#include <vector>
#include "Hash.h"

class DumbHash : public Hash {

    int k;

    // Rolling k-mer
    std::vector<uint8_t> kmer;

    int i = 0;

    static constexpr int size = 10000;

public:
    DumbHash(int k, const std::vector<uint8_t> &start);

    void update(uint8_t c) override;
};


#endif //MTF_DUMBHASH_H
