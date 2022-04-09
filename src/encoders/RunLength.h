
#ifndef MTF_RUNLENGTH_H
#define MTF_RUNLENGTH_H

#include "stream/obitstream/obitstream.h"
#include "AdaptiveHuffman.h"

class RunLength {

    AdaptiveHuffman ahrle;
    AdaptiveHuffman ah;
    int n;

    int counter;
    int last;
    int remaining;
    bool eof;

public:
    explicit RunLength(int alphabet_size, int n = 4);

    void encode_array(const uint32_t *data, int length, obitstream& out);

    void encode_end(uint32_t eof_symbol, obitstream& out);

    int decode_array(ibitstream& in, uint32_t *data, int length, uint32_t eof_symbol);
};

#endif //MTF_RUNLENGTH_H
