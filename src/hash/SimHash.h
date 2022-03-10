
#ifndef MTF_SIMHASH_H
#define MTF_SIMHASH_H


#include <vector>
#include "Hash.h"

class SimHash : public Hash {
private:
    std::vector<std::vector<uint8_t>> vectors;

    void compute();

public:

    explicit SimHash(int k);

    void update(uint8_t c) override;

    void init(const std::vector<uint8_t> &start) override;

    void resize(uint64_t size) override;
};


#endif //MTF_SIMHASH_H
