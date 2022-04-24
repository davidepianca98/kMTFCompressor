
#include <cassert>
#include "TabulationHash.h"

uint64_t TabulationHash::get_random_uint64() {
    return (((uint64_t) dis(gen) <<  0) & 0x000000000000FFFFull) |
            (((uint64_t) dis(gen) << 16) & 0x00000000FFFF0000ull) |
            (((uint64_t) dis(gen) << 32) & 0x0000FFFF00000000ull) |
            (((uint64_t) dis(gen) << 48) & 0xFFFF000000000000ull);
}

TabulationHash::TabulationHash(int k, uint64_t seed) : Hash(k, seed), last_index(k - 1) {
    assert(k <= MAX_KMER);

    for (int j1 = 0; j1 < k; j1++) {
        for (int j2 = 0; j2 < LEN; j2++) {
            T[j1][j2] = get_random_uint64();
        }
    }
}

uint8_t TabulationHash::update(uint8_t c) {
    uint8_t old = kmer_hash_p[last_index];
    kmer_hash -= (uint64_t) old << (last_index * 8);
    kmer_hash = (kmer_hash << 8) | c;

    hash = 0;
    int j;
    for (j = 0; j < k; j++) {
        hash ^= T[j][kmer_hash_p[last_index - j]];
    }
    hash &= 0x7FFFFFFFull;
    return old;
}
