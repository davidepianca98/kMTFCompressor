
#include <cmath>
#include "MTFRankBuffer.h"

template <uint32_t SIZE>
void MTFRankBuffer<SIZE>::normalize_rank_counter() {
    amount++;
    if (amount >= 4096) {
        for (auto & c : counter) {
            c = (uint64_t) log2((double) (c + 1));
        }
        amount = 0;
    }
}

template <uint32_t SIZE>
void MTFRankBuffer<SIZE>::shift(uint8_t c, uint8_t i) {
    counter[i]++;

    for (int j = i - 1; j >= 0; j--) {
        if (counter[i] > counter[j]) {
            std::swap(counter[i], counter[j]);

            // Swap
            std::swap(MTFBuffer<SIZE>::buffer[i], MTFBuffer<SIZE>::buffer[j]);

            i = j;
        } else {
            // Because the list is ordered
            break;
        }
    }

    normalize_rank_counter();
}

template <uint32_t SIZE>
void MTFRankBuffer<SIZE>::append(uint8_t c) {
    uint32_t i;
    for (i = 0; i < SIZE - 1; i++) {
        if (counter[i] < 1) {
            break;
        }
    }

    counter[i] = 1;
    MTFBuffer<SIZE>::buffer[i] = c;
    if (MTFBuffer<SIZE>::symbols < SIZE) {
        MTFBuffer<SIZE>::symbols++;
    }

    normalize_rank_counter();
}

template class MTFRankBuffer<2>;
template class MTFRankBuffer<4>;
template class MTFRankBuffer<6>;
template class MTFRankBuffer<8>;
template class MTFRankBuffer<16>;
template class MTFRankBuffer<32>;
template class MTFRankBuffer<64>;
template class MTFRankBuffer<128>;
template class MTFRankBuffer<256>;
