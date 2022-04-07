
#ifndef MTF_MTFBUFFER_H
#define MTF_MTFBUFFER_H

#include <cstdint>
#include <boost/multiprecision/cpp_int.hpp>

template <typename T>
class MTFBuffer {
protected:
    T buf = 0;

    uint8_t symbols = 0;
    bool is_visited = false;

public:
    constexpr static uint8_t byte_size() noexcept {
        if (std::is_same<T, boost::multiprecision::uint128_t>::value) {
            return 16;
        } else if (std::is_same<T, boost::multiprecision::uint256_t>::value) {
            return 32;
        } else if (std::is_same<T, boost::multiprecision::uint512_t>::value) {
            return 64;
        } else if (std::is_same<T, boost::multiprecision::uint1024_t>::value) {
            return 128;
        } else {
            return sizeof(T);
        }
    }

    virtual void shift(uint8_t c, uint8_t i) {
        // If the position is zero, no need to change the buffer
        if (i != 0) {
            int bits = (i + 1) * 8;
            T left = (buf >> bits) << bits; // Extract the part to be preserved
            bits = (byte_size() - i) * 8;
            buf = (buf << bits) >> (bits - 8); // Make space for the character in the first position and clean the leftmost bytes
            buf |= left | c; // Put character in the first position
        }
    }

    virtual void append(uint8_t c) {
        buf = (buf << 8) | c;
        if (symbols < byte_size()) {
            symbols++;
        }
    }

    uint8_t extract(uint8_t i) {
        return static_cast<uint8_t>((buf >> (i * 8)) & 0xFF);
    }

    uint32_t encode(uint8_t c) {
        for (uint8_t i = 0; i < symbols; i++) {
            uint8_t extracted = extract(i);
            if (extracted == c) { // Check if the character in the i-th position from the right is equal to c
                shift(c, i);
                return i;
            }
        }

        // Not found so add the symbol to the buffer
        append(c);

        // Sum byte_size to differentiate between indexes on the MTF buffer and characters
        return c + byte_size();
    }

    uint8_t decode(uint32_t symbol) {
        uint8_t c;
        if (symbol >= byte_size()) {
            c = symbol - byte_size();
            append(c);
        } else {
            c = extract(symbol);
            shift(c, symbol);
        }
        return c;
    }

    T get_buf() {
        return buf;
    }

    inline bool visited() {
        return is_visited;
    }

    inline void set_visited() {
        is_visited = true;
    }

};

#endif //MTF_MTFBUFFER_H
