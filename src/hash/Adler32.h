
#ifndef MTF_ADLER32_H
#define MTF_ADLER32_H

#include <vector>
#include "Hash.h"

class Adler32 : public Hash {
private:
    uint64_t i = 0;
    uint32_t a;
    uint32_t b;

    static constexpr int BASE = 65521;

public:
    explicit Adler32(int k, int size) : Hash(k, k, size) {}

    void init(const std::vector<uint8_t> &start) override {
        a = 1;
        b = 0;
        // First k-mer
        for (int j = 0; j < k; j++) {
            uint8_t c = start[j];
            kmer[j] = c;

            a += c % BASE;
            b += a % BASE;
        }

        hash = (b << 16) | a;
    }

    void resize(uint64_t size) override {
        this->size = size;
    }

    void update(uint8_t c) override {
        // Update k-mer
        uint8_t old = kmer[i];
        kmer[i] = c;
        i = (i + 1) % k;

        b = (a >> 16) & 0xFFFF;
        a &= 0xFFFF;

        a = (a - old + c) % BASE;
        b = (b - (k * old) + a) % BASE;

        hash = (b << 16) | a;
    }

    [[nodiscard]] uint64_t get_hash() const override {
        // Resize for the table size
        return Hash::get_hash() % size;
    }

    [[nodiscard]] uint64_t get_hash_full() const {
        return Hash::get_hash();
    }
};

#endif //MTF_ADLER32_H
