
#include "DumbHash.h"

DumbHash::DumbHash(int k): Hash(100000, k) {}

void DumbHash::init(const std::vector<uint8_t> &start) {
    // First k-mer
    for (i = 0; i < k; i++) {
        kmer[i] = start[i];
        hash += kmer[i] << (i * 8);
    }

    hash = hash % size;
}

void DumbHash::resize(uint64_t size) {
    // TODO
}

void DumbHash::update(uint8_t c) {
    kmer[i] = c;
    i = (i + 1) % k;

    hash >>= 8;
    hash += c << ((k - 1) * 8); // TODO assumes k <= 8
    hash = hash % size;
}
