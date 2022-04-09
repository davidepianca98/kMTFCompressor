
#include "Hash.h"

Hash::Hash(int k, uint64_t seed) : k(k), kmer(k, 0), gen(seed), dis(0, 1000000) {}

uint64_t Hash::fast_modulo(uint64_t val) {
    uint64_t res = (val & M61) + (val >> 61);
    return (res >= M61) ? res - M61 : res;
}

void Hash::update(uint8_t c) {

}

uint64_t Hash::get_hash() const {
    return hash;
}

uint64_t Hash::get_length() const {
    return k;
}
