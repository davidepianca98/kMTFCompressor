
#include <utility>
#include "MinimiserHash.h"

uint32_t MinimiserHash::fnv1a(int start, int len) {
    uint32_t hash = 2166136261;
    for (int s = start; s < len; s++) {
        hash ^= window[s % window_size];
        hash *= 16777619;
    }
    return hash;
}

void MinimiserHash::compute() {
    uint32_t minimum = UINT32_MAX;
    // For all k-mers compute hash
    for (i = 0; i < window_size - k; i++) {
        window_hashes[i] = fnv1a(j, k);
        //window_hashes[i] = rf.get_hash();
        if (window_hashes[i] < minimum) {
            minimum = window_hashes[i];
            // TODO compute f(window.data() + i, k)
            hash = fnv1a(j, k) << 8;
            //last_update = i;
        }
        j++;
        //rf.update(window[i + k]);
    }
    // TODO compute f'(window) and update hash
    hash = hash | (fnv1a(0, window_size) % 256);
    hash = hash % size;
}

MinimiserHash::MinimiserHash(int k, int window_size, const std::vector<uint8_t>& start) :
        Hash(10000), rf(k, start), k(k), window(start), window_size(window_size), window_hashes(window_size - k) {

    hash = UINT32_MAX;
    compute();
}

void MinimiserHash::update(uint8_t c) {
    i = (i + 1) % (window_size - k);
    j = (j + 1) % window_size;
    window[j] = c;
    window_hashes[i] = fnv1a(j, k);

    uint32_t minimum = UINT32_MAX;
    for (int i = 0; i < window_size - k; i++) {
        if (window_hashes[i] < minimum) {
            minimum = window_hashes[i];
            // TODO compute f(window.data() + i, k)
            hash = fnv1a(i, k) << 8;
            //last_update = i;
        }
        j++;
    }
    hash = hash | (fnv1a((j + 1) % window_size, window_size) % 256);
    hash = hash % size;
}
