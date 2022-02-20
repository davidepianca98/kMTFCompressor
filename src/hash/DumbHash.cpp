
#include "DumbHash.h"

DumbHash::DumbHash(int k, const std::vector<uint8_t>& start): k(k), kmer(k) {
    // First k-mer
    for (i = 0; i < k; i++) {
        kmer.at(i) = start.at(i);
    }

    for (uint8_t c: start) {
        hash += c;
    }
    hash = hash % size;
}

void DumbHash::update(uint8_t c) {
    uint8_t old = kmer.at(i % k);
    kmer.at(i % k) = c;
    i++;

    hash -= old;
    hash += c;
    hash = hash % size;
}
