

#ifndef MTF_MINIMISERHASH_H
#define MTF_MINIMISERHASH_H


#include <vector>
#include "Hash.h"
#include "RabinFingerprint.h"

class MinimiserHash : public Hash {
    int k;
    int window_size;

    std::vector<uint8_t> window;
    std::vector<uint64_t> window_hashes;

    RabinFingerprint rf;

    int i = 0;
    int filled = 0;

    uint32_t fnv1a(int start, int len);

public:
    MinimiserHash(int k, int window_size);

    void init(const std::vector<uint8_t> &start) override;

    void resize(uint64_t size) override;

    void update(uint8_t c) override;
};


#endif //MTF_MINIMISERHASH_H
