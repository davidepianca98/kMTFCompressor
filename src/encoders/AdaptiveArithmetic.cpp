
#include "AdaptiveArithmetic.h"

AdaptiveArithmetic::AdaptiveArithmetic(uint32_t alphabet_size) : alphabet_size(alphabet_size) {
    low = 0;
    high = UINT32_MAX;

    underflow = 0;

    for (uint32_t i = 0; i <= alphabet_size; i++) {
        cumulative[i] = alphabet_size - i + 1;
        frequencies[i] = 1;

        symbol_to_index[i] = i + 1;
        index_to_symbol[i + 1] = i;
    }
    frequencies[0] = 0;
}

void AdaptiveArithmetic::update(int index, obitstream& out) {
    uint64_t range = high - low;
    high = low + (cumulative[index - 1] * range / cumulative[0]);
    low = low + (cumulative[index] * range / cumulative[0]);

    uint32_t low_top_bit = low >> TOP_BIT;
    while (low_top_bit == high >> TOP_BIT) {
        out.write_bit(low_top_bit);

        while (underflow > 0) {
            out.write_bit(!low_top_bit);
            underflow--;
        }

        low = low << 1;
        high = (high << 1) | 1;
        low_top_bit = low >> TOP_BIT;
    }

    while ((low & ~high & SECOND_TOP_BIT_MASK) != 0) {
        underflow++;
        // Shift and invert top bit
        low = (low << 1) ^ TOP_BIT_MASK;
        // Shift, set top and bottom bit
        high = (high << 1) | TOP_BIT_MASK | 1;
    }
}

void AdaptiveArithmetic::encode(uint32_t symbol, obitstream &out) {
    int index = (int) symbol_to_index[symbol];
    update(index, out);

    int i;
    if (cumulative[0] == MAXIMUM) {
        uint64_t sum = 0;
        for (i = (int) alphabet_size; i >= 0; i--) {
            frequencies[i] = (frequencies[i] + 1) / 2;
            cumulative[i] = sum;
            sum += frequencies[i];
        }
    }
    for (i = index; frequencies[i] == frequencies[i - 1]; i--);
    if (i < index) {
        uint32_t old_index = index_to_symbol[i];
        uint32_t old_symbol = index_to_symbol[index];
        index_to_symbol[i] = old_symbol;
        index_to_symbol[index] = old_index;
        symbol_to_index[old_index] = index;
        symbol_to_index[old_symbol] = i;
    }
    frequencies[i]++;
    while (i > 0) {
        i--;
        cumulative[i]++;
    }
}

void AdaptiveArithmetic::encode_end(uint32_t eof_symbol, obitstream &out) {
    encode(eof_symbol, out);
    out.write_bit(1);
}

void AdaptiveArithmetic::encode_array(const uint32_t *data, int length, obitstream &out) {
    for (int i = 0; i < length; i++) {
        encode(data[i], out);
    }
}
