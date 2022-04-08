
#ifndef MTF_MINIMISERHASH_H
#define MTF_MINIMISERHASH_H

#include "Hash.h"

/*
 * The idea is to select the kmer with the minimum hash1 in the window, apply hash2 to it and concatenate a small hash
 * of the whole window, so that the left part of the hash remains equal for a few iterations and the right offset
 * changes giving a different location very close to the previous to avoid cache misses.
 */
template <typename HASH1, typename HASH2, typename HASH3>
class MinimiserHash : public Hash {
    std::vector<uint64_t> window_hashes1;
    std::vector<uint64_t> window_hashes2;

    HASH1 hash1;
    HASH2 hash2;
    HASH3 hash_window;

    int i = 0;
    int filled;
    int min_index;
    uint64_t minimum;

    static constexpr int sub_k = 3;

public:
    MinimiserHash(int k, uint64_t seed);

    void update(uint8_t c) override;
};


#endif //MTF_MINIMISERHASH_H
