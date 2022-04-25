
#include <cassert>
#include "Hash.h"

Hash::Hash(int k, uint64_t seed) : k(k), last_index(k - 1), gen(seed), dis(0, 1000000) {
    assert(k <= 8);
    hash = 0;
}

uint8_t Hash::update(uint8_t c) {
    uint8_t old = kmer_hash_p[last_index];
    kmer_hash -= (uint64_t) old << (last_index * 8);
    kmer_hash = (kmer_hash << 8) | c;

    return old;
}

void Hash::increment_k() {
    if (k < 8) {
        k++;
        last_index++;
    }
}

uint64_t Hash::compute(uint64_t key) {
    return key;
}
