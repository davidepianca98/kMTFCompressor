
#include "VectorHash.h"

VectorHash::VectorHash(int k, int size) : Hash(k, size) {}

void VectorHash::update(uint8_t c) {
    // Update k-mer
    uint8_t old = kmer[i];
    kmer[i] = c;

    // Faster than wrapping with modulo
    i++;
    if (i >= k) {
        i = 0;
    }
    sum -= old;
    sum += c;

    // Sum is the dot product with the unit diagonal vector (1, ..., 1)
    // k is the same as the dot product of the unit vector with itself
    hash = (uint64_t) (((double) sum / (double) (k * 255)) * max_size);
}
