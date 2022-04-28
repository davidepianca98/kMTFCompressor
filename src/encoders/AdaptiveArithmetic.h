
#ifndef MTF_ADAPTIVEARITHMETIC_H
#define MTF_ADAPTIVEARITHMETIC_H

#include <iostream>
#include "stream/ibitstream/ibitstream.h"
#include "stream/obitstream/obitstream.h"

class AdaptiveArithmetic {

    static constexpr int MAX_ALPHA_SIZE = 256 + 128;
    uint64_t cumulative[MAX_ALPHA_SIZE + 1] = { 0 };
    uint64_t frequencies[MAX_ALPHA_SIZE + 1] = { 0 };
    uint32_t index_to_symbol[MAX_ALPHA_SIZE] = { 0 };
    uint32_t symbol_to_index[MAX_ALPHA_SIZE] = { 0 };

    static constexpr int FRACTION_BITS = 32;
    static constexpr uint32_t TOP_BIT = FRACTION_BITS - 1;
    static constexpr uint32_t TOP_BIT_MASK = 1 << TOP_BIT;
    static constexpr uint32_t SECOND_TOP_BIT_MASK = TOP_BIT_MASK >> 1;
    static constexpr uint32_t MAXIMUM = UINT32_MAX / 4;

    uint32_t low;
    uint32_t high;

    uint64_t underflow;

    uint32_t alphabet_size;

    void update(int index, obitstream& out);

public:
    explicit AdaptiveArithmetic(uint32_t alphabet_size);

    void encode(uint32_t symbol, obitstream& out);

    void encode_array(const uint32_t *data, int length, obitstream& out);

    void encode_end(uint32_t eof_symbol, obitstream& out);
};


#endif //MTF_ADAPTIVEARITHMETIC_H
