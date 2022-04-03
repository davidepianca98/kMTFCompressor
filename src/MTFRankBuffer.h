
#ifndef MTF_MTFRANKBUFFER_H
#define MTF_MTFRANKBUFFER_H

#include <cstdint>
#include "MTFBuffer.h"

template <typename T>
class MTFRankBuffer : public MTFBuffer<T> {

    std::vector<uint64_t> counter;

public:
    MTFRankBuffer() : counter(this->byte_size(), 0) {}

    void shift(uint8_t c, uint8_t i) override {
        counter[i]++;

        for (int j = i - 1; j >= 0; j--) {
            if (counter[i] > counter[j]) {
                std::swap(counter[i], counter[j]);

                uint8_t c2 = MTFBuffer<T>::extract(j);

                // Swap
                T max = 0xFF;
                this->buf = (this->buf & ~(max << (i * 8))) | ((T) c2 << (i * 8));
                this->buf = (this->buf & ~(max << (j * 8))) | ((T) c << (j * 8));

                i = j;
            } else {
                // Because the list is ordered
                break;
            }
        }
    }

    void append(uint8_t c) override {
        int i;
        for (i = 0; i < this->byte_size() - 1; i++) {
            if (counter[i] < 1) {
                break;
            }
        }
        counter[i] = 1;
        if (i == 0) {
            MTFBuffer<T>::append(c);
        } else {
            T right = (this->buf << ((this->byte_size() - i) * 8)) >> ((this->byte_size() - i) * 8);
            // Insert in position i
            this->buf = right | ((T) c << (i * 8));
        }
    }

    void normalizeRankCounter(std::vector<uint64_t>& count) {
        for (auto & c : count) {
            c = (uint64_t) log2((double) (c + 1));
        }
    }

};

#endif //MTF_MTFRANKBUFFER_H