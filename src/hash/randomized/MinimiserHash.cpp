
#include "MinimiserHash.h"
#include "RabinKarp.h"
#include "LinearHash.h"

template <typename HASH1, typename HASH2, typename HASH3>
MinimiserHash<HASH1, HASH2, HASH3>::MinimiserHash(int k, uint64_t seed) : Hash(k, seed), hash1(sub_k, seed), hash2(sub_k, seed), hash_window(k, seed),
                                                     window_hashes1(k - sub_k + 1), window_hashes2(k - sub_k + 1), filled(0),
                                                     min_index(0), minimum(UINT64_MAX) {}

template <typename HASH1, typename HASH2, typename HASH3>
void MinimiserHash<HASH1, HASH2, HASH3>::update(uint8_t c) {
    hash1.update(c);
    hash2.update(c);
    window_hashes1[i] = hash1.get_hash();
    window_hashes2[i] = hash2.get_hash();
    hash_window.update(c);

    if (filled < k - sub_k + 1) {
        filled++;
    }

    if (min_index == i) {
        // Go through the entire list only if the last minimum just went out of the window
        minimum = window_hashes1[0];
        min_index = 0;
        for (int j = 1; j < filled; j++) {
            if (window_hashes1[j] < minimum) {
                min_index = j;
                minimum = window_hashes1[min_index];
            }
        }
    } else if (window_hashes1[i] < minimum) {
        min_index = i;
        minimum = window_hashes1[min_index];
    }

    hash = window_hashes2[min_index];

    hash = (hash << 3) | (hash_window.get_hash() & 7);

    i = (i + 1) % (k - sub_k + 1);
}

template class MinimiserHash<RabinKarp, LinearHash, RabinKarp>;
