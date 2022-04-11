
#include "Hash.h"

Hash::Hash(int k, uint64_t seed) : k(k), kmer(k, 0), gen(seed), dis(0, 1000000) {
    hash = 0;
    i = 0;
}

uint64_t Hash::fast_modulo(uint64_t val) {
    uint64_t res = (val & M61) + (val >> 61);
    return (res >= M61) ? res - M61 : res;
}

uint8_t Hash::update(uint8_t c) {
    // Update k-mer
    uint8_t old = kmer[i];
    kmer[i] = c;

    // Faster than wrapping with modulo
    i++;
    if (i >= k) {
        i = 0;
    }
    return old;
}

uint64_t Hash::get_hash() const {
    return hash;
}

uint64_t Hash::get_length() const {
    return k;
}
