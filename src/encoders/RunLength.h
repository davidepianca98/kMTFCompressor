
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

public:
    explicit RunLength(int alphabet_size, int n = 4);

    void encode_array(const uint32_t *data, int length, obitstream& out);

    void encode(uint32_t symbol, obitstream& out);

    int decode_array(ibitstream& in, uint32_t *data, int length, uint32_t eof);
};

#endif //MTF_RUNLENGTH_H
