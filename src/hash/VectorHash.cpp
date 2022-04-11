
#include "VectorHash.h"

VectorHash::VectorHash(int k, int size) : Hash(k, size) {}

uint8_t VectorHash::update(uint8_t c) {
    uint8_t old = Hash::update(c);
    sum -= old;
    sum += c;

    // Sum is the dot product with the unit diagonal vector (1, ..., 1)
    // k is the same as the dot product of the unit vector with itself
    hash = (uint64_t) (((double) sum / (double) (k * 255)) * max_size);
    return old;
}
