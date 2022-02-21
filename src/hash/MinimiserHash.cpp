
#include "MinimiserHash.h"

uint32_t MinimiserHash::fnv1a() {
    uint32_t hash = 2166136261;
    for (int j = i; j < k; j++) {
        hash = hash ^ kmer.at(j % k);
        hash = hash * 16777619;
    }
    return hash % size;
}

MinimiserHash::MinimiserHash(
        int k,
        int window_size,
        const std::vector<uint8_t>& start
        ):
        Hash(RabinFingerprint::q),
        k(k),
        window_size(window_size),
        rf(k, start),
        kmer(k),
        window_hashes(window_size) {
    hash = UINT64_MAX;
    // The window contains the k-mer hashes
    for (i = 0; i < window_size - k; i++) {
        window_hashes.at(i) = rf.get_hash();
        if (window_hashes.at(i) < hash) {
            hash = window_hashes.at(i);
            last_update = i;
        }
        rf.update(start.at(i + k));
    }
}

void MinimiserHash::update(uint8_t c) {
    rf.update(c);
    i++;

    window_hashes.at(i % window_size) = rf.get_hash();
    if (window_hashes.at(i % window_size) < hash) {
        hash = window_hashes.at(i % window_size);
        last_update = i;
    }
    // If the last hash is now out of the window, search the new minimum
    if (i - last_update > window_size) {
        hash = window_hashes.at(0);
        for (int j = 1; j < window_size; j++) {
            if (window_hashes.at(j) < hash) {
                hash = window_hashes.at(j);
            }
        }
        last_update = i;
    }
}
