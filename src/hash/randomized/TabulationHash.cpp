
#include <cassert>
#include "TabulationHash.h"

uint64_t TabulationHash::get_random_uint64() {
    return (((uint64_t) dis(gen) <<  0) & 0x000000000000FFFFull) |
            (((uint64_t) dis(gen) << 16) & 0x00000000FFFF0000ull) |
            (((uint64_t) dis(gen) << 32) & 0x0000FFFF00000000ull) |
            (((uint64_t) dis(gen) << 48) & 0xFFFF000000000000ull);
}

TabulationHash::TabulationHash(int k, uint64_t seed) : Hash(k, seed) {
    assert(k <= MAX_KMER);

    for (int j1 = 0; j1 < MAX_KMER; j1++) {
        for (int j2 = 0; j2 < LEN; j2++) {
            T[j1][j2] = get_random_uint64();
        }
    }
}

uint8_t TabulationHash::update(uint8_t c) {
    uint8_t old = Hash::update(c);

    hash = 0;
    for (int j = 0; j < k; j++) {
        hash ^= T[j][(kmer_hash >> (last_index - j) * 8) & 0xFF];
    }
    hash &= 0x7FFFFFFF;
    return old;
}

// Duplicated because update is much faster without this function call
uint64_t TabulationHash::compute(uint64_t key) {
    uint64_t res = 0;
    for (int j = 0; j < k; j++) {
        res ^= T[j][(key >> (last_index - j) * 8) & 0xFF];
    }
    res &= 0x7FFFFFFF;
    return res;
}
