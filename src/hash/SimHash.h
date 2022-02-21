
#ifndef MTF_SIMHASH_H
#define MTF_SIMHASH_H


#include <vector>
#include "Hash.h"

class SimHash : public Hash {
private:
    int k;

    // Rolling k-mer
    std::vector<uint8_t> kmer;

    std::vector<std::vector<uint8_t>> vectors;

public:

    SimHash(int k, const std::vector<uint8_t> &start);

    void update(uint8_t c) override;

    void compute();
};


#endif //MTF_SIMHASH_H
