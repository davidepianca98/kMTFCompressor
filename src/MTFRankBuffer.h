
#ifndef MTF_MTFRANKBUFFER_H
#define MTF_MTFRANKBUFFER_H

#include <cstdint>
#include "MTFBuffer.h"

template <typename T>
class MTFRankBuffer : public MTFBuffer<T> {

    uint64_t counter[MTFBuffer<T>::byte_size()] = { 0 };
    int amount = 0;

    void normalize_rank_counter() {
        amount++;
        if (amount >= 4096) { // TODO maybe more than 4096
            for (auto & c : counter) {
                c = (uint64_t) log2((double) (c + 1));
            }
            amount = 0;
        }
    }

public:

    void shift(uint8_t c, uint8_t i) override {
        counter[i]++;

        for (int j = i - 1; j >= 0; j--) {
            if (counter[i] > counter[j]) {
                std::swap(counter[i], counter[j]);

                uint8_t c2 = MTFBuffer<T>::extract(j);

                // Swap
                T max = 0xFF;
                MTFBuffer<T>::buf = (MTFBuffer<T>::buf & ~(max << (i * 8))) | ((T) c2 << (i * 8));
                MTFBuffer<T>::buf = (MTFBuffer<T>::buf & ~(max << (j * 8))) | ((T) c << (j * 8));

                i = j;
            } else {
                // Because the list is ordered
                break;
            }
        }

        normalize_rank_counter();
    }

    void append(uint8_t c) override {
        int i;
        for (i = 0; i < MTFBuffer<T>::byte_size() - 1; i++) {
            if (counter[i] < 1) {
                break;
            }
        }
        counter[i] = 1;
        if (i == 0) {
            MTFBuffer<T>::append(c);
        } else {
            T right = (MTFBuffer<T>::buf << ((MTFBuffer<T>::byte_size() - i) * 8)) >> ((MTFBuffer<T>::byte_size() - i) * 8);
            // Insert in position i
            MTFBuffer<T>::buf = right | ((T) c << (i * 8));

            if (MTFBuffer<T>::symbols < MTFBuffer<T>::byte_size()) {
                MTFBuffer<T>::symbols++;
            }
        }

        normalize_rank_counter();
    }

};

#endif //MTF_MTFRANKBUFFER_H
