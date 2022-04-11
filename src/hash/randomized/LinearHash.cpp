
#include "LinearHash.h"

LinearHash::LinearHash(int k, uint64_t seed) : Hash(k, seed) {
    a = dis(gen);
    b = dis(gen);
    hash = 0;
    kmer_hash = 0;
}

uint8_t LinearHash::update(uint8_t c) {
    uint8_t old = Hash::update(c);

    kmer_hash -= old << ((k - 1) * 8);
    kmer_hash = (kmer_hash << 8) | c;

    hash = (a * kmer_hash + b) % M61;

    return old;
}
