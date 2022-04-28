
#ifndef MTF_MTFBUFFER_H
#define MTF_MTFBUFFER_H

#include <cstdint>

template <uint32_t SIZE>
class MTFBuffer {
protected:

    uint64_t key = 0;

    uint8_t buffer[SIZE] = { 0 };

    // Leftmost bit is visited or not, remaining 7 bits are the amount of symbols in the buffer
    uint8_t symbols = 0;

public:

    void shift(uint8_t i) {
        uint8_t c = buffer[i];
        // If the position is zero, no need to change the buffer
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

    inline uint8_t extract(uint8_t i) {
        return buffer[i];
    }

    [[nodiscard]] inline bool is_visited() const {
        return (bool) (symbols >> 7);
    }

    void set_visited(uint64_t key) {
        symbols |= (1 << 7);
        this->key = key;
    }

    [[nodiscard]] inline uint64_t get_key() const {
        return key;
    }

    [[nodiscard]] inline uint8_t get_size() const {
        return symbols & 0x7F;
    }

};

#endif //MTF_MTFBUFFER_H
