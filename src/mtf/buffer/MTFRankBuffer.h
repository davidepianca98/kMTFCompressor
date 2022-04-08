
#ifndef MTF_MTFRANKBUFFER_H
#define MTF_MTFRANKBUFFER_H

#include <cstdint>
#include "MTFBuffer.h"

template <typename T>
class MTFRankBuffer : public MTFBuffer<T> {

    uint32_t counter[MTFBuffer<T>::byte_size()] = { 0 };
    int amount = 0;

    void normalize_rank_counter();

public:

    void shift(uint8_t c, uint8_t i) override;

    void append(uint8_t c) override;

};

#endif //MTF_MTFRANKBUFFER_H
