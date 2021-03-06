
#ifndef MTF_IDENTITY_H
#define MTF_IDENTITY_H

#include "Hash.h"

class Identity : public Hash {
private:
    uint64_t sh = 0;

public:
    Identity(int k, uint64_t seed);

    uint8_t update(uint8_t c) override;
};

#endif //MTF_IDENTITY_H
