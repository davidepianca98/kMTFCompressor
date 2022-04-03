
#ifndef MTF_MTFBUFFER_H
#define MTF_MTFBUFFER_H

#include <cstdint>
#include <boost/multiprecision/cpp_int.hpp>

template <typename T>
class MTFBuffer {
protected:
    T buf = 0;

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
    }

    uint8_t extract(uint8_t i) {
        return static_cast<uint8_t>((buf >> (i * 8)) & 0xFF);
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
