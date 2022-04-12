
#ifndef MTF_MTFBUFFER_H
#define MTF_MTFBUFFER_H

#include <cstdint>

template <uint32_t SIZE>
class MTFBuffer {
protected:
    uint8_t buffer[SIZE] = { 0 };

    uint8_t symbols = 0;

    uint64_t key = 0;

public:

    virtual void shift(uint8_t i) {
        uint8_t c = buffer[i];
        // If the position is zero, no need to change the buffer
        for (int j = i; j > 0; j--) {
            buffer[j] = buffer[j - 1];
        }
        buffer[0] = c;
    }

    virtual void append(uint8_t c) {
        if (symbols >= SIZE) {
            symbols--;
        }
        for (int j = symbols; j > 0; j--) {
            buffer[j] = buffer[j - 1];
        }
        buffer[0] = c;
        symbols++;
    }

    uint8_t extract(uint8_t i) {
        return buffer[i];
    }

    uint32_t encode(uint8_t c) {
        for (uint8_t i = 0; i < symbols; i++) {
            uint8_t extracted = extract(i);
            if (extracted == c) { // Check if the character in the i-th position from the right is equal to c
                shift(i);
                return i;
            }
        }

        // Not found so add the symbol to the buffer
        append(c);

        // Sum size to differentiate between indexes on the MTF buffer and characters
        return c + SIZE;
    }

    uint8_t decode(uint32_t symbol) {
        uint8_t c;
        if (symbol >= SIZE) {
            c = symbol - SIZE;
            append(c);
        } else {
            c = extract(symbol);
            shift(symbol);
        }
        return c;
    }

    bool visited() {
        return symbols > 0;
    }

    void set_visited(uint64_t hash) {
        key = hash;
    }

    uint64_t get_key() {
        return key;
    }

};

#endif //MTF_MTFBUFFER_H
