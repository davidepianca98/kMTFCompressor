
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

    virtual void shift(uint8_t c, uint8_t i);

    virtual void append(uint8_t c);

    uint8_t extract(uint8_t i);

    uint32_t encode(uint8_t c);

    uint8_t decode(uint32_t symbol);

    T get_buf();

    bool visited();

    void set_visited();

};

#endif //MTF_MTFBUFFER_H
