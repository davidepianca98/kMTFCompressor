
#include "RabinKarp.h"

RabinKarp::RabinKarp(int k, uint64_t seed) : Hash(k, seed) {
    base = dis(gen);

    // Build the multiplier (power) for the leftmost character, needed to remove it when updating
    xk = 1;
    for (int j = 0; j < k - 1; j++) {
        xk = fast_modulo(xk * base);
    }
}

uint8_t RabinKarp::update(uint8_t c) {
    uint8_t old = Hash::update(c);

    // Remove the leftmost character using the multiplier
    hash = fast_modulo(hash - (fast_modulo(xk * old)));
    // Shift left by the multiplier
    hash = fast_modulo(hash * base);
    // Push the new character to the right
    hash = fast_modulo(hash + c);
    return old;
}
