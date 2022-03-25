
#ifndef MTF_ADAPTIVEELIASGAMMA_H
#define MTF_ADAPTIVEELIASGAMMA_H

#include <cstdint>
#include <numeric>
#include "bitstream.h"

class AdaptiveEliasGamma {
private:
    std::vector<uint64_t> count; // Keeps track of the frequencies of the symbols, indexed by symbol
    std::vector<uint32_t> rank; // Keeps track of the rank of the symbol, indexed by symbol
    std::vector<uint32_t> map; // Keeps track of the symbol by rank, indexed by rank

    std::vector<int> length;

public:
    explicit AdaptiveEliasGamma(int byte_size) : count(256 + byte_size, 0), rank(256 + byte_size, 0), map(256 + byte_size, 0) {
        std::iota(std::begin(rank), std::end(rank), 0);
        std::iota(std::begin(map), std::end(map), 0);

        for (int i = 1; i <= 256 + byte_size; i++) {
            length.push_back(1 + (int) floor(log2(i)));
        }
    }

    void update_frequencies(uint32_t num) {
        count[num]++;

        int i = rank[num];
        int j = i;

        while (j > 0 && count[num] > count[map[j - 1]]) {
            j--;
        }

        std::swap(rank[num], rank[map[j]]);
        std::swap(map[i], map[j]);
    }

    void encode(uint32_t num, obitstream& out) {
        num--;

        // First encode and then move, otherwise not reversible
        elias_gamma_encode(rank[num] + 1, out);

        update_frequencies(num);
    }

    int decode(ibitstream& in) {
        int num = elias_gamma_decode(in);
        if (num < 0) {
            return num;
        }

        num--;
        int val = map[num] + 1;

        update_frequencies(val - 1);

        return val;
    }

    void elias_gamma_encode(uint32_t num, obitstream& out) {
        int len = length[num - 1];

        for (int k = 0; k < len - 1; k++)
            out.writeBit(0);
        for (int k = len - 1; k >= 0; k--)
            out.writeBit((num >> k) & 1);
    }

    static int elias_gamma_decode(ibitstream& in) {
        int num = 1;
        int n = 0;

        if (!in.remaining()) {
            return -1;
        }

        while (in.readBit() == 0) {
            n++;
        }
        for (int j = 0; j < n; j++) {
            num <<= 1;
            num |= in.readBit();
        }

        return num;
    }

    void elias_delta_encode(uint32_t num, obitstream& out) {
        int len = length[num - 1];
        int lengthOfLen = floor(log2(len));

        for (int k = lengthOfLen; k > 0; --k)
            out.writeBit(0);
        for (int k = lengthOfLen; k >= 0; --k)
            out.writeBit((len >> k) & 1);
        for (int k = len - 2; k >= 0; k--)
            out.writeBit((num >> k) & 1);
    }

    static int elias_delta_decode(ibitstream& in) {
        int num = 1;
        int len = 1;
        int lengthOfLen = 0;
        while (in.readBit() == 0) {
            lengthOfLen++;
        }
        if (!in.remaining()) {
            return -1;
        }
        for (int j = 0; j < lengthOfLen; j++) {
            len <<= 1;
            if (in.readBit() == 1)
                len |= 1;
        }
        for (int j = 0; j < len - 1; j++) {
            num <<= 1;
            if (in.readBit() == 1)
                num |= 1;
        }
        return num;
    }
};

#endif //MTF_ADAPTIVEELIASGAMMA_H
