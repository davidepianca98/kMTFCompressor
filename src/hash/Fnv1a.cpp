
#include "Fnv1a.h"

Fnv1a::Fnv1a(int k, uint64_t seed) : Hash(k, seed) {}

uint8_t Fnv1a::update(uint8_t c) {
    uint8_t old = Hash::update(c);

    uint32_t hash = BASE;
    for (int j = 0; j < k; j++) {
        hash = (hash ^ kmer[(i + j + 1) % k]) * PRIME;
    }
    Hash::hash = hash;
    return old;
}
