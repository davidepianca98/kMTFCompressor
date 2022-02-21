
#include <vector>
#include "VectorHash.h"

uint64_t VectorHash::dot(std::vector<uint8_t> u, std::vector<uint8_t> v) {
    uint64_t result = 0;
    for (int i = 0; i < u.size(); i++) {
        result += u[i] * v[i];
    }
    return result;
}

void VectorHash::compute() {
    uint64_t a = dot(kmer, v);

    hash = (uint64_t) (((double) a / (double) b) * size);
}

VectorHash::VectorHash(int k, const std::vector<uint8_t> &start) : Hash(10000), v(k), kmer(k) {
    for (uint8_t& c: v) {
        c = 255;
    }
    b = dot(v, v);

    for (int i = 0; i < k; i++) {
        kmer.at(i) = start.at(i);
    }
    compute();
}

void VectorHash::update(uint8_t c) {
    for (int i = 0; i < kmer.size() - 1; i++) {
        kmer[i] = kmer[i + 1];
    }
    kmer[kmer.size() - 1] = c;
    compute();
}
