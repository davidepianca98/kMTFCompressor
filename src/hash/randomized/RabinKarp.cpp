
#include "RabinKarp.h"

RabinKarp::RabinKarp(int k, uint64_t seed) : Hash(k, seed) {
    base = dis(gen);

    // Build the multiplier (power) for the leftmost character, needed to remove it when updating
    xk = 1;
    for (int j = 0; j < k - 1; j++) {
        xk = (xk * base) % M61;
    }
}

void RabinKarp::update(uint8_t c) {
    // Update k-mer
    uint8_t old = kmer[i];
    kmer[i] = c;

    // Faster than wrapping with modulo
    i++;
    if (i >= k) {
        i = 0;
    }

    // Remove the leftmost character using the multiplier
    hash = (hash - ((xk * old) % M61)) % M61;
    // Shift left by the multiplier
    hash = (hash * base) % M61;
    // Push the new character to the right
    hash = (hash + c) % M61;
}
