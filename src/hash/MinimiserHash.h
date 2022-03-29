

#ifndef MTF_MINIMISERHASH_H
#define MTF_MINIMISERHASH_H


#include <vector>
#include <cmath>
#include "Hash.h"
#include "RabinKarp.h"

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

    HASH1 hash_kmer;
    HASH2 hash_kmer2;
    HASH3 hash_window;

    int i = 0;
    int filled;
    int min_index;
    uint64_t minimum;

    int first_size_log;
    int sh;

public:
    MinimiserHash(int k, int window_size, int size) : Hash(k, window_size, size), hash_kmer(k, size),
                                    hash_kmer2(k, size), hash_window(window_size, size), window_hashes(window_size),
                                    window_hashes2(window_size),
                                    first_size_log((int) log2(size) - 2), sh((int) log2((int) log2(size) - first_size_log)) {}

    void init(const std::vector<uint8_t> &start) override {
        hash = size;

        window = start;
        hash_kmer.init(start);
        hash_kmer2.init(start);
        hash_window.init(start);
        window_hashes[0] = hash_kmer.get_hash();
        window_hashes2[0] = hash_kmer2.get_hash();
        filled = 1;

        minimum = window_hashes[0];
        min_index = 0;
        hash = hash_kmer2.get_hash_full();
        // For all k-mers compute hash
        for (i = 1; i < window_size - k + 1; i++) {
            hash_kmer.update(window[i + k - 1]);
            hash_kmer2.update(window[i + k - 1]);

            window_hashes[i] = hash_kmer.get_hash();
            window_hashes2[i] = hash_kmer2.get_hash();

            if (window_hashes[i] < minimum) {
                minimum = window_hashes[i];
                min_index = i;
                hash = window_hashes2[i];
            }
            filled++;
        }

        hash = (hash << sh) | (hash_window.get_hash_full() & (~0ul >> (64 - sh)));
        hash = hash % size;
    }

    void resize(uint64_t size) override {
        this->size = size;
        sh = (int) log2((int) log2(size) - first_size_log);
    }

    void update(uint8_t c) override {
        window[(i + k - 1) % window_size] = c;
        hash_kmer.update(c);
        hash_kmer2.update(c);
        window_hashes[i] = hash_kmer.get_hash();
        window_hashes2[i] = hash_kmer2.get_hash();
        hash_window.update(c);

        if (filled < window_size) {
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

        hash = (hash << sh) | (hash_window.get_hash_full() & (~0ul >> (64 - sh)));
        hash = hash % size;
        i = (i + 1) % window_size;
    }
};


#endif //MTF_MINIMISERHASH_H
