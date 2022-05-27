
#ifndef MTF_ADAPTIVEARITHMETIC_H
#define MTF_ADAPTIVEARITHMETIC_H

#include <iostream>
#include "stream/ibitstream/ibitstream.h"
#include "stream/obitstream/obitstream.h"

class AdaptiveArithmetic {

    static constexpr int MAX_ALPHA_SIZE = 256 + 256 + 1;
    uint32_t cumulative[MAX_ALPHA_SIZE + 1] = { 0 };
    uint32_t frequencies[MAX_ALPHA_SIZE + 1] = { 0 };
    uint32_t index_to_symbol[MAX_ALPHA_SIZE] = { 0 };
    uint32_t symbol_to_index[MAX_ALPHA_SIZE] = { 0 };

    static constexpr int FRACTION_BITS = 32;
    static constexpr uint32_t TOP_BIT = FRACTION_BITS - 1;
    static constexpr uint32_t TOP_BIT_MASK = 1 << TOP_BIT;
    static constexpr uint32_t SECOND_TOP_BIT_MASK = TOP_BIT_MASK >> 1;
    static constexpr uint32_t MAXIMUM = UINT32_MAX / 4;

    uint32_t low;
    uint32_t high;

    uint32_t code;

    uint64_t underflow;

    uint32_t alphabet_size;

    bool eof = false;

    void update(int index, obitstream& out);

    void update_decode(int index, ibitstream& in);

    void update_distribution(int index);

public:
    explicit AdaptiveArithmetic(uint32_t alphabet_size);

    void encode(uint32_t symbol, obitstream& out);

    void encode_array(const uint32_t *data, int length, obitstream& out);

    void encode_end(uint32_t eof_symbol, obitstream& out);

    void start_decode(ibitstream& in);

    uint32_t decode(ibitstream& in);

    int decode_array(ibitstream& in, uint32_t *data, int length, uint32_t eof_symbol);
};


#endif //MTF_ADAPTIVEARITHMETIC_H
