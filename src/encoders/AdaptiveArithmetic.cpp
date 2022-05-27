
#include "AdaptiveArithmetic.h"

AdaptiveArithmetic::AdaptiveArithmetic(uint32_t alphabet_size) : alphabet_size(alphabet_size) {
    low = 0;
    high = UINT32_MAX;

    underflow = 0;

    code = 0;

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

void AdaptiveArithmetic::update_distribution(int index) {
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

void AdaptiveArithmetic::encode(uint32_t symbol, obitstream &out) {
    int index = (int) symbol_to_index[symbol];
    update(index, out);
    update_distribution(index);
}

void AdaptiveArithmetic::encode_end(uint32_t eof_symbol, obitstream &out) {
    encode(eof_symbol, out);
    out.write_bit(1);
}

void AdaptiveArithmetic::encode_array(const uint32_t *data, int length, obitstream& out) {
    for (int i = 0; i < length; i++) {
        encode(data[i], out);
    }
}

void AdaptiveArithmetic::start_decode(ibitstream& in) {
    for (int i = 0; i < FRACTION_BITS; i++)
        code = code << 1 | in.read_bit();
}

void AdaptiveArithmetic::update_decode(int index, ibitstream& in) {
    uint64_t range = high - low;
    high = low + (cumulative[index - 1] * range / cumulative[0]);
    low = low + (cumulative[index] * range / cumulative[0]);

    uint32_t low_top_bit = low >> TOP_BIT;
    while (low_top_bit == high >> TOP_BIT) {
        code = (code << 1) | in.read_bit();

        low = low << 1;
        high = (high << 1) | 1;
        low_top_bit = low >> TOP_BIT;
    }

    while ((low & ~high & SECOND_TOP_BIT_MASK) != 0) {
        code = (code & SECOND_TOP_BIT_MASK) | ((code << 1) & (UINT32_MAX >> 1)) | in.read_bit();
        // Shift and invert top bit
        low = (low << 1) ^ TOP_BIT_MASK;
        // Shift, set top and bottom bit
        high = (high << 1) | TOP_BIT_MASK | 1;
    }
}

uint32_t AdaptiveArithmetic::decode(ibitstream& in) {
    if (cumulative[0] > MAXIMUM)
        throw std::invalid_argument("Cannot decode symbol because total is too large");
    uint64_t range = high - low;
    uint64_t offset = code - low;
    uint64_t value = ((offset + 1) * cumulative[0] - 1) / range;
    if (value * range / cumulative[0] > offset)
        throw std::logic_error("Assertion error");
    if (value >= cumulative[0])
        throw std::logic_error("Assertion error");

    uint32_t start = 0;
    uint32_t end = alphabet_size - 1;
    while (end - start > 1) {
        uint32_t middle = (start + end) >> 1;
        if (cumulative[middle] > value)
            end = middle;
        else
            start = middle;
    }
    if (start + 1 != end)
        throw std::logic_error("Assertion error");

    int index = start;
    if (offset < cumulative[index] * range / cumulative[0] || cumulative[index - 1] * range / cumulative[0] <= offset)
        throw std::logic_error("Assertion error");
    update_decode(index, in);
    update_distribution(index);
    if (code < low || code > high)
        throw std::logic_error("Assertion error: Code out of range");
    return index_to_symbol[index];
}

int AdaptiveArithmetic::decode_array(ibitstream& in, uint32_t *data, int length, uint32_t eof_symbol) {
    if (eof) {
        return 0;
    }
    int decompressed_size = 0;
    while (decompressed_size < length) {
        int value = decode(in);
        if (value == -1 || value == (int) eof_symbol) {
            eof = true;
            return decompressed_size;
        }
        data[decompressed_size++] = value;
    }
    return decompressed_size;
}
