
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
    explicit Adler32(int k, int size) : Hash(k, size) {}

    void init(const std::vector<uint8_t> &start) override {
        a = 1;
        b = 0;
        // First k-mer
        for (int j = 0; j < k; j++) {
            uint8_t c = start[j];
            kmer[j] = c;

            a = (a + c) % BASE;
            b = (b + a) % BASE;
        }

        hash = (b << 16) | a;
    }

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
