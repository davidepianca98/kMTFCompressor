
#include <cassert>
#include "Identity.h"

Identity::Identity(int k, uint64_t seed) : Hash(k, seed) {
    assert(k <= 8);
    sh = (~0ul >> (64 - (8 * k)));
    hash = 0;
}

uint8_t Identity::update(uint8_t c) {
    uint8_t old = Hash::update(c);

    hash = (hash << 8) | c;
    hash &= sh;

    return old;
}
