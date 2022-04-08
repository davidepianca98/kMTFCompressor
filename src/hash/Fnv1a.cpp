
#include "Fnv1a.h"

Fnv1a::Fnv1a(int k, uint64_t seed) : Hash(k, seed) {}

void Fnv1a::update(uint8_t c) {
    // Update k-mer
    kmer[i] = c;
    i = (i + 1) % k;

    uint32_t hash = BASE;
    for (int j = 0; j < k; j++) {
        hash = (hash ^ kmer[(i + j + 1) % k]) * PRIME;
    }
    Hash::hash = hash;
}
