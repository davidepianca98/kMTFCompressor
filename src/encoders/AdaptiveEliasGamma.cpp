
#include <cmath>
#include "AdaptiveEliasGamma.h"

AdaptiveEliasGamma::AdaptiveEliasGamma(int symbols_amount) : count(symbols_amount, 0), rank(symbols_amount, 0), map(symbols_amount, 0) {
    std::iota(std::begin(rank), std::end(rank), 0);
    std::iota(std::begin(map), std::end(map), 0);
}

void AdaptiveEliasGamma::update_frequencies(uint32_t symbol) {
    count[symbol]++;

    int i = rank[symbol];
    int j = i;

    while (j > 0 && count[symbol] > count[map[j - 1]]) {
        j--;
    }

    std::swap(rank[symbol], rank[map[j]]);
    std::swap(map[i], map[j]);
}

void AdaptiveEliasGamma::encode(uint32_t symbol, obitstream& out) {
    symbol--;

    // First encode and then move, otherwise not reversible
    elias_gamma_encode(rank[symbol] + 1, out);

    update_frequencies(symbol);
}

void AdaptiveEliasGamma::encode_array(const uint32_t *data, int size, obitstream& out) {
    for (int i = 0; i < size; i++) {
        encode(data[i] + 1, out);
    }
}

int AdaptiveEliasGamma::decode(ibitstream& in) {
    int symbol = elias_gamma_decode(in);
    if (symbol < 0) {
        return symbol;
    }

    symbol--;
    int val = map[symbol] + 1;

    update_frequencies(val - 1);

    return val;
}

int AdaptiveEliasGamma::decode_array(ibitstream& in, uint32_t *data, int size) {
    int decompressed_size = 0;
    while (decompressed_size < size) {
        int value = decode(in);
        if (value == -1) {
            return decompressed_size;
        }
        data[decompressed_size++] = value - 1;
    }
    return decompressed_size;
}

void AdaptiveEliasGamma::elias_gamma_encode(uint32_t symbol, obitstream& out) {
    int len = 1 + fast_log2(symbol);

    for (int k = 0; k < len - 1; k++)
        out.write_bit(0);
    for (int k = len - 1; k >= 0; k--)
        out.write_bit((symbol >> k) & 1);
}

int AdaptiveEliasGamma::elias_gamma_decode(ibitstream& in) {
    int symbol = 1;
    int n = 0;

    if (!in.remaining()) {
        return -1;
    }

    while (in.read_bit() == 0) {
        n++;
    }
    for (int j = 0; j < n; j++) {
        symbol <<= 1;
        int bit = in.read_bit();
        if (bit == -1) {
            return -1;
        }
        symbol |= bit;
    }

    return symbol;
}

void AdaptiveEliasGamma::elias_delta_encode(uint32_t symbol, obitstream& out) {
    int len = 1 + fast_log2(symbol);
    int lengthOfLen = fast_log2(len);

    for (int k = lengthOfLen; k > 0; --k)
        out.write_bit(0);
    for (int k = lengthOfLen; k >= 0; --k)
        out.write_bit((len >> k) & 1);
    for (int k = len - 2; k >= 0; k--)
        out.write_bit((symbol >> k) & 1);
}

int AdaptiveEliasGamma::elias_delta_decode(ibitstream& in) {
    int symbol = 1;
    int len = 1;
    int length_of_len = 0;
    while (in.read_bit() == 0) {
        length_of_len++;
    }
    if (!in.remaining()) {
        return -1;
    }
    for (int j = 0; j < length_of_len; j++) {
        len <<= 1;
        int bit = in.read_bit();
        if (bit == -1) {
            return -1;
        } else {
            len |= bit;
        }
    }
    for (int j = 0; j < len - 1; j++) {
        symbol <<= 1;
        int bit = in.read_bit();
        if (bit == -1) {
            return -1;
        } else {
            len |= bit;
        }
    }
    return symbol;
}
