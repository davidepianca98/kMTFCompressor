
#ifndef MTF_MTFRANKBUFFER_H
#define MTF_MTFRANKBUFFER_H

#include <cstdint>
#include "MTFBuffer.h"

template <uint32_t SIZE>
class MTFRankBuffer : public MTFBuffer<SIZE> {

    uint32_t counter[SIZE] = { 0 };
    int amount = 0;

    void normalize_rank_counter();

public:

    void shift(uint8_t c, uint8_t i) override;

    void append(uint8_t c) override;

};

#endif //MTF_MTFRANKBUFFER_H
