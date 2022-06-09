
#ifndef MTF_MTFBUFFER_H
#define MTF_MTFBUFFER_H

#include <cstdint>

template <uint32_t SIZE>
class MTFBuffer {
protected:

    uint8_t buffer[SIZE] = { 0 };

    uint32_t key = 0x80000000;

    uint8_t symbols = 0;

public:

    void shift(uint8_t i) {
        uint8_t c = buffer[i];
        for (int j = i; j > 0; j--) {
            buffer[j] = buffer[j - 1];
        }
        buffer[0] = c;
    }

    void append(uint8_t c) {
        if (get_size() >= SIZE) {
            symbols--;
        }
        for (int j = get_size(); j > 0; j--) {
            buffer[j] = buffer[j - 1];
        }
        buffer[0] = c;
        symbols++;
    }

    // SIMD search of symbol c
    int search(uint8_t c) {
        const __m256i symbol_vec = _mm256_set1_epi8((char) c);
        __m256i vec = _mm256_lddqu_si256((const __m256i *) &buffer);
        __m256i eq = _mm256_cmpeq_epi8(vec, symbol_vec);

        __m256i res = _mm256_or_si256(eq, vec);
        // Set corresponding bit in mask if most significant bit of 8-bit integer is set, which means a match or an
        // empty slot has been found
        int mask = _mm256_movemask_epi8(res);
        if (mask != 0) {
            uint32_t i = _lzcnt_u32(mask);
            if (i < symbols) {
                shift(i);
                return i;
            }
        }
        return -1;
    }

    inline uint8_t extract(uint8_t i) {
        return buffer[i];
    }

    [[nodiscard]] inline bool is_visited() const {
        return !((bool) (key >> 31));
    }

    void set_visited(uint64_t key) {
        this->key = key;
    }

    [[nodiscard]] inline uint64_t get_key() const {
        return key;
    }

    [[nodiscard]] inline uint16_t get_size() const {
        return symbols;
    }

};

#endif //MTF_MTFBUFFER_H
