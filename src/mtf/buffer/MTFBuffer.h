
#ifndef MTF_MTFBUFFER_H
#define MTF_MTFBUFFER_H

#include <cstdint>

template <uint32_t SIZE>
class MTFBuffer {
protected:
    uint8_t buffer[SIZE] = { 0 };

    uint8_t symbols = 0;
    bool is_visited = false;

    uint64_t key = 0;

public:

    virtual void shift(uint8_t c, uint8_t i);

    virtual void append(uint8_t c);

    uint8_t extract(uint8_t i);

    uint32_t encode(uint8_t c);

    uint8_t decode(uint32_t symbol);

    bool visited();

    void set_visited(uint64_t hash);

    uint64_t get_key();

};

#endif //MTF_MTFBUFFER_H
