
#include "RabinKarp.h"

RabinKarp::RabinKarp(int k, uint64_t seed) : Hash(k, seed) {
    base = dis(gen);

    // Build the multiplier (power) for the leftmost character, needed to remove it when updating
    xk = 1;
    for (int j = 0; j < k - 1; j++) {
        xk = (xk * base) % M61;
    }
}

uint8_t RabinKarp::update(uint8_t c) {
    uint8_t old = Hash::update(c);

    // Remove the leftmost character using the multiplier
    hash = (hash - ((xk * old) % M61)) % M61;
    // Shift left by the multiplier
    hash = (hash * base) % M61;
    // Push the new character to the right
    hash = (hash + c) % M61;
    return old;
}
