
#ifndef MTF_FNV1A_H
#define MTF_FNV1A_H

#include "Hash.h"

class Fnv1a : public Hash {
private:
    static constexpr uint64_t BASE = 2166136261;
    //static constexpr uint64_t BASE = 14695981039346656037u;

    static constexpr uint64_t PRIME = 16777619;
    //static constexpr uint64_t PRIME = 1099511628211;

public:
    Fnv1a(int k, uint64_t seed);

    void update(uint8_t c) override;
};

#endif //MTF_FNV1A_H
