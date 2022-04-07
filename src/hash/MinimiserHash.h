
#ifndef MTF_MINIMISERHASH_H
#define MTF_MINIMISERHASH_H

#include <vector>
#include "Hash.h"

/*
 * The idea is to select the kmer with the minimum hash1 in the window, apply hash2 to it and concatenate a small hash
 * of the whole window, so that the left part of the hash remains equal for a few iterations and the right offset
 * changes giving a different location very close to the previous to avoid cache misses.
 */
template <typename HASH1, typename HASH2, typename HASH3>
class MinimiserHash : public Hash {
    std::vector<uint8_t> window;
    std::vector<uint64_t> window_hashes;
    std::vector<uint64_t> window_hashes2;

    HASH1 hash1;
    HASH2 hash2;
    HASH3 hash_window;

    int i = 0;
    int filled;
    int min_index;
    uint64_t minimum;

    static constexpr int sub_k = 2;


public:
    MinimiserHash(int k, uint64_t seed) : Hash(k, seed), hash1(sub_k, seed), hash2(sub_k, seed), hash_window(k, seed),
                                    window_hashes(k), window_hashes2(k) {}

    void init(const std::vector<uint8_t> &start) override {
        window = start;
        hash1.init(start);
        hash2.init(start);
        hash_window.init(start);
        window_hashes[0] = hash1.get_hash();
        window_hashes2[0] = hash2.get_hash();
        filled = 1;

        minimum = window_hashes[0];
        min_index = 0;
        hash = hash2.get_hash();
        // For all k-mers compute hash
        for (i = 1; i < k - sub_k + 1; i++) {
            hash1.update(window[i + sub_k - 1]);
            hash2.update(window[i + sub_k - 1]);

            window_hashes[i] = hash1.get_hash();
            window_hashes2[i] = hash2.get_hash();

            if (window_hashes[i] < minimum) {
                minimum = window_hashes[i];
                min_index = i;
                hash = window_hashes2[i];
            }
            filled++;
        }

        hash = (hash << 3) | (hash_window.get_hash() & 7);
    }

    void update(uint8_t c) override {
        window[(i + sub_k - 1) % k] = c;
        hash1.update(c);
        hash2.update(c);
        window_hashes[i] = hash1.get_hash();
        window_hashes2[i] = hash2.get_hash();
        hash_window.update(c);

        if (filled < k) {
            filled++;
        }

        if (min_index == i) {
            // Go through the entire list only if the last minimum just went out of the window
            minimum = window_hashes[0];
            min_index = 0;
            hash = window_hashes2[0];
            for (int s = 1; s < filled; s++) {
                if (window_hashes[s] < minimum) {
                    min_index = s;
                    minimum = window_hashes[min_index];
                    hash = window_hashes2[min_index];
                }
            }
        } else if (window_hashes[i] < minimum) {
            min_index = i;
            minimum = window_hashes[min_index];
            hash = window_hashes2[min_index];
        } else {
            hash = window_hashes2[min_index];
        }

        hash = (hash << 3) | (hash_window.get_hash() & 7);
        i = (i + 1) % k;
    }
};


#endif //MTF_MINIMISERHASH_H
