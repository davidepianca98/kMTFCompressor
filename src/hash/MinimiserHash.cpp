
#include <cmath>
#include "MinimiserHash.h"

uint32_t MinimiserHash::fnv1a(int start, int len) {
    uint32_t hash = 2166136261;
    for (int s = start; s < start + len; s++) {
        hash ^= window[s % window_size];
        hash *= 16777619;
    }
    return hash;
}

MinimiserHash::MinimiserHash(int k, int window_size) :
        Hash(1024, k), rf(k), k(k), window_size(window_size), window_hashes(window_size) {}



void MinimiserHash::init(const std::vector<uint8_t> &start) {
    hash = size;

    window = start;
    rf.init(start);
    window_hashes[0] = rf.get_hash();
    filled++;

    /*uint64_t minimum = window_hashes[0];
    hash = fnv1a(0, k) % size;
    // For all k-mers compute hash
    for (i = 1; i < window_size - k + 1; i++) {
        rf.update(window[i + k - 1]);

        window_hashes[i] = rf.get_hash();

        if (window_hashes[i] < minimum) {
            minimum = window_hashes[i];
            hash = fnv1a(i, k) % size;
        }
        filled++;
    }*/

    hash = fnv1a(i, k);

    //hash = (hash << 8) | (fnv1a(0, window_size) % 0xFF);
    int sh = (int) log2(size) - 9;
    hash = (hash << sh) | (fnv1a(0, window_size) % ~(~0u << sh));
    hash = hash % size;
}

void MinimiserHash::update(uint8_t c) {
    window[(i + k - 1) % window_size] = c;
    /*rf.update(c);
    window_hashes[i] = rf.get_hash();

    if (filled < window_size) {
        filled++;
    }

    uint64_t minimum = window_hashes[0];
    hash = fnv1a(0, k) % size;
    for (int s = 1; s < filled; s++) { // TODO don't pass the whole list, only check the newest if it's lower than current minimum and pass all list only if minimum just went out of the window
        if (window_hashes[s] < minimum) {
            minimum = window_hashes[s];
            hash = fnv1a(s, k) % size;
        }
    }*/
    hash = fnv1a(i, k);

    //hash = (hash << 8) | (fnv1a((i + k) % window_size, window_size) % 0xFF); TODO this works well with pizzachili, much smaller shift works well with calgary, so I'm trying it dynamic with table size but weird that fills exactly to 50%
    int sh = (int) log2(size) - 9;
    hash = (hash << sh) | (fnv1a((i + k), window_size) % ~(~0u << sh));
    hash = hash % size;
    i = (i + 1) % window_size;
}

void MinimiserHash::resize(uint64_t size) {
    this->size = size;
}
