
#include "TabulationHash.h"

TabulationHash::TabulationHash(int k, uint64_t seed) : Hash(k, seed), p(k * 8) {
    t = ceil((double) p / (double) r);
    len = (int) pow(2, (double) r);
    T = new uint64_t[len * t];

    for (int j1 = 0; j1 < t; j1++) {
        for (int j2 = 0; j2 < len; j2++) {
            T[j1 * len + j2] = dis(gen);
        }
    }
}

TabulationHash::~TabulationHash() {
    delete[] T;
}

void TabulationHash::update(uint8_t c) {
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

    hash = 0;
    for (int j = 0; j < t; j++) {
        hash ^= T[j * len + (kmer_hash >> (r * j)) & 0xFF]; // TODO instead of FF should be r ones
    }
}