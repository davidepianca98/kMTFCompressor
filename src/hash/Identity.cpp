
#include <cassert>
#include "Identity.h"

Identity::Identity(int k, uint64_t seed) : Hash(k, seed) {
    assert(k <= 8);
    sh = (~0ul >> (64 - (8 * k)));
    hash = 0;
}

void Identity::update(uint8_t c) {
    hash = (hash << 8) | c;
    hash &= sh;
}
