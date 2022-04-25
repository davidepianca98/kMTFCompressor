
#include "Identity.h"

Identity::Identity(int k, uint64_t seed) : Hash(k, seed) {
    hash = 0;
}

uint8_t Identity::update(uint8_t c) {
    uint8_t old = Hash::update(c);
    hash = kmer_hash;
    return old;
}
