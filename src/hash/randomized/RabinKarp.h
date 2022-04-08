
#ifndef MTF_RABINKARP_H
#define MTF_RABINKARP_H


#include "Hash.h"

class RabinKarp : public Hash {
private:
    // Base
    uint64_t base;
    // Multiplier to shift left (depends on k)
    uint64_t xk = 1;

    uint64_t i = 0;

public:
    RabinKarp(int k, uint64_t seed);

    void update(uint8_t c) override;
};


#endif //MTF_RABINKARP_H