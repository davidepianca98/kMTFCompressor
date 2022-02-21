
#include "RabinFingerprint.h"


RabinFingerprint::RabinFingerprint(int k, const std::vector<uint8_t> &start) : Hash(RabinFingerprint::q), k(k), kmer(k) {
    //srand(time(nullptr));
    // Multiplier
    //x = rand() % q;
    x = 3980; // TODO choose in another way, it must be always the same, to be able to decompress later, so no random

    // First k-mer
    for (int i = 0; i < k; i++) {
        uint8_t c = start.at(i);
        kmer.at(i) = c;

        // Multiply the hash by the multiplier to "shift left"
        hash = (hash * x) % q;
        // Add the new character (push right)
        hash = (hash + c) % q;
    }

    // Build the multiplier (power) for the leftmost character, needed to remove it when updating
    for (int j = 0; j < k - 1; j++) {
        xk = (xk * x) % q;
    }
}

void RabinFingerprint::update(uint8_t c) {
    // Update k-mer
    uint8_t old = kmer.at(i);
    kmer.at(i) = c;
    i = (i + 1) % k;


    hash += q;
    // Remove the leftmost character using the multiplier
    hash = (hash - ((xk * old) % q)) % q;
    // Shift left by the multiplier
    hash = (hash * x) % q;
    // Push the new character to the right
    hash = (hash + c) % q;
}
