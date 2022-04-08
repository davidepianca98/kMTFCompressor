
#include "LinearHash.h"

LinearHash::LinearHash(int k, uint64_t seed) : Hash(k, seed) {
    a = dis(gen);
    b = dis(gen);
    hash = 0;
    kmer_hash = 0;
}

void LinearHash::update(uint8_t c) {
    // Update k-mer
    uint8_t old = kmer[i];
    kmer[i] = c;

    // Faster than wrapping with modulo
    i++;
    if (i >= k) {
        i = 0;
    }

    kmer_hash -= old << ((k - 1) * 8);
    kmer_hash = (kmer_hash << 8) | c;

    hash = (a * kmer_hash + b) % M61;
}
