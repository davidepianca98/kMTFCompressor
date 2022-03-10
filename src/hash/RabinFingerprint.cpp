
#include <iostream>
#include "RabinFingerprint.h"


RabinFingerprint::RabinFingerprint(int k) : Hash(q, k) {} // TODO should be passed from table

void RabinFingerprint::init(const std::vector<uint8_t> &start) {
    //srand(time(nullptr));
    // Multiplier
    //x = rand() % q;
    x = 3980; // TODO choose in another way, it must be always the same, to be able to decompress later, so no random

    // First k-mer
    for (int i = 0; i < k; i++) {
        uint8_t c = start[i];
        kmer[i] = c;

        // Multiply the hash by the multiplier to "shift left"
        hash = (hash * x) % q;
        // Add the new character (push right)
        hash = (hash + c) % q;
    }

    // Build the multiplier (power) for the leftmost character, needed to remove it when updating
    for (int j = 0; j < k - 1; j++) {
        xk = (xk * x) % q;
    }

    // Resize for the table size
    //hash %= size;
}

void RabinFingerprint::update(uint8_t c) {
    // Update k-mer
    uint8_t old = kmer[i];
    kmer[i] = c;
    i = (i + 1) % k;


    hash += q;
    // Remove the leftmost character using the multiplier
    hash = (hash - ((xk * old) % q)) % q;
    // Shift left by the multiplier
    hash = (hash * x) % q;
    // Push the new character to the right
    hash = (hash + c) % q;

    // Resize for the table size
    //hash %= size;
}

void RabinFingerprint::resize(uint64_t size) {
    this->size = size;
}

void RabinFingerprint::increment_k(uint8_t c) {
    k++;
    std::cout << k << std::endl;
    kmer.resize(k);
    for (uint64_t j = k - 1; j > i; j--) {
        kmer[j] = kmer[j - 1];
    }
    kmer[i] = c;
    hash = (hash * x) % size;
    hash = (hash + c) % size;

    i = (i + 1) % k;

    xk = (xk * x) % size;
}

void RabinFingerprint::decrement_k() {
    k--;
    // TODO
    xk = (xk / x) % size;
}
