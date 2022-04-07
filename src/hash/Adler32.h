
#ifndef MTF_ADLER32_H
#define MTF_ADLER32_H

#include <vector>
#include "Hash.h"

class Adler32 : public Hash {
private:
    uint64_t i = 0;
    uint32_t a = 1;
    uint32_t b = 0;

    static constexpr int BASE = 65521;

public:
    Adler32(int k, uint64_t seed) : Hash(k, seed) {}

    void update(uint8_t c) override {
        // Update k-mer
        uint8_t old = kmer[i];
        kmer[i] = c;
        i = (i + 1) % k;

        a = (a - old + c) % BASE;
        b = (b - ((k - 1) * old) + a) % BASE;

        hash = (b << 16) | a;
    }
};

#endif //MTF_ADLER32_H
