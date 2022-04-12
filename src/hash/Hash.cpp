
#include "Hash.h"

Hash::Hash(int k, uint64_t seed) : k(k), kmer(k, 0), gen(seed), dis(0, 1000000) {
    hash = 0;
    i = 0;
}

uint8_t Hash::update(uint8_t c) {
    // Update k-mer
    uint8_t old = kmer[i];
    kmer[i] = c;

    i = (i + 1) % k;
    return old;
}
