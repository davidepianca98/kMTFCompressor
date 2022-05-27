
#ifndef MTF_COUNTBUFFER_H
#define MTF_COUNTBUFFER_H

#include <cstdint>
#include <cmath>
#include "MTFBuffer.h"

template <uint32_t SIZE>
class CountBuffer {

    uint8_t buffer[SIZE] = { 0 };

    uint16_t counter[SIZE] = { 0 };
    uint16_t amount = 0;

    uint32_t key = 0x80000000;

    uint8_t symbols = 0;

    inline uint32_t fast_log2(uint32_t x) {
        return 31 - __builtin_clz(x);
    }

    void normalize_rank_counter() {
        amount++;
        if (amount >= 4096) {
            amount = 0;
            for (auto & c : counter) {
                c = (uint16_t) log2((double) (c + 1));
                amount += c;
            }
        }
    }

public:

    void shift(uint8_t i) {
        counter[i]++;

        for (int j = i - 1; j >= 0; j--) {
            if (counter[i] > counter[j]) {
                std::swap(counter[i], counter[j]);
                std::swap(buffer[i], buffer[j]);
                i = j;
            } else {
                // Because the list is ordered
                break;
            }
        }

        normalize_rank_counter();
    }

    void append(uint8_t c) {
        uint32_t i;
        if (symbols < SIZE) {
            symbols++;
            for (i = 0; i < SIZE - 1; i++) {
                if (counter[i] < 1) {
                    break;
                }
            }
        } else {
            i = SIZE - 1;
        }

        amount -= counter[i];
        counter[i] = 1;
        buffer[i] = c;

        normalize_rank_counter();
    }

    inline uint8_t extract(uint8_t i) {
        return buffer[i];
    }

    [[nodiscard]] bool is_visited() const {
        return !((bool) (key >> 31));
    }

    void set_visited(uint64_t key) {
        this->key = key;
    }

    [[nodiscard]] uint64_t get_key() const {
        return key;
    }

    [[nodiscard]] uint8_t get_size() const {
        return symbols;
    }

};

#endif //MTF_COUNTBUFFER_H
