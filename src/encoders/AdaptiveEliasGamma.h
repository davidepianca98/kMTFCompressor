
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
    explicit AdaptiveEliasGamma(int symbols_amount) : count(symbols_amount, 0), rank(symbols_amount, 0), map(symbols_amount, 0) {
        std::iota(std::begin(rank), std::end(rank), 0);
        std::iota(std::begin(map), std::end(map), 0);

        for (int i = 1; i <= symbols_amount; i++) {
            length.push_back(1 + (int) floor(log2(i)));
        }
    }

    void update_frequencies(uint32_t symbol) {
        count[symbol]++;

        int i = rank[symbol];
        int j = i;

        while (j > 0 && count[symbol] > count[map[j - 1]]) {
            j--;
        }

        std::swap(rank[symbol], rank[map[j]]);
        std::swap(map[i], map[j]);
    }

    void encode(uint32_t symbol, obitstream& out) {
        symbol--;

        // First encode and then move, otherwise not reversible
        elias_gamma_encode(rank[symbol] + 1, out);

        update_frequencies(symbol);
    }

    int decode(ibitstream& in) {
        int symbol = elias_gamma_decode(in);
        if (symbol < 0) {
            return symbol;
        }

        symbol--;
        int val = map[symbol] + 1;

        update_frequencies(val - 1);

        return val;
    }

    void elias_gamma_encode(uint32_t symbol, obitstream& out) {
        int len = length[symbol - 1];

        for (int k = 0; k < len - 1; k++)
            out.writeBit(0);
        for (int k = len - 1; k >= 0; k--)
            out.writeBit((symbol >> k) & 1);
    }

    static int elias_gamma_decode(ibitstream& in) {
        int symbol = 1;
        int n = 0;

        if (!in.remaining()) {
            return -1;
        }

        while (in.readBit() == 0) {
            n++;
        }
        for (int j = 0; j < n; j++) {
            symbol <<= 1;
            symbol |= in.readBit();
        }

        return symbol;
    }

    void elias_delta_encode(uint32_t symbol, obitstream& out) {
        int len = length[symbol - 1];
        int lengthOfLen = floor(log2(len));

        for (int k = lengthOfLen; k > 0; --k)
            out.writeBit(0);
        for (int k = lengthOfLen; k >= 0; --k)
            out.writeBit((len >> k) & 1);
        for (int k = len - 2; k >= 0; k--)
            out.writeBit((symbol >> k) & 1);
    }

    static int elias_delta_decode(ibitstream& in) {
        int symbol = 1;
        int len = 1;
        int length_of_len = 0;
        while (in.readBit() == 0) {
            length_of_len++;
        }
        if (!in.remaining()) {
            return -1;
        }
        for (int j = 0; j < length_of_len; j++) {
            len <<= 1;
            if (in.readBit() == 1)
                len |= 1;
        }
        for (int j = 0; j < len - 1; j++) {
            symbol <<= 1;
            if (in.readBit() == 1)
                symbol |= 1;
        }
        return symbol;
    }
};

#endif //MTF_ADAPTIVEELIASGAMMA_H
