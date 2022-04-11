
#ifndef MTF_MTFRANKBUFFER_H
#define MTF_MTFRANKBUFFER_H

#include <cstdint>
#include <cmath>
#include "MTFBuffer.h"

template <uint32_t SIZE>
class MTFRankBuffer : public MTFBuffer<SIZE> {

    uint32_t counter[SIZE] = { 0 };
    int amount = 0;

    void normalize_rank_counter() {
        amount++;
        if (amount >= 4096) {
            for (auto & c : counter) {
                c = (uint64_t) log2((double) (c + 1));
            }
            amount = 0;
        }
    }

public:

    void shift(uint8_t i) override {
        counter[i]++;

        for (int j = i - 1; j >= 0; j--) {
            if (counter[i] > counter[j]) {
                std::swap(counter[i], counter[j]);
                std::swap(MTFBuffer<SIZE>::buffer[i], MTFBuffer<SIZE>::buffer[j]);
                i = j;
            } else {
                // Because the list is ordered
                break;
            }
        }

        normalize_rank_counter();
    }

    void append(uint8_t c) override {
        uint32_t i;
        if (MTFBuffer<SIZE>::symbols < SIZE) {
            MTFBuffer<SIZE>::symbols++;
            for (i = 0; i < SIZE - 1; i++) {
                if (counter[i] < 1) {
                    break;
                }
            }
        } else {
            i = SIZE - 1;
        }

        counter[i] = 1;
        MTFBuffer<SIZE>::buffer[i] = c;

        normalize_rank_counter();
    }

};

#endif //MTF_MTFRANKBUFFER_H
