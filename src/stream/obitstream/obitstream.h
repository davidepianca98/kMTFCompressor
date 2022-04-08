
#ifndef MTF_OBITSTREAM_H
#define MTF_OBITSTREAM_H

#include <ostream>
#include <vector>

class obitstream: public std::ostream {
private:
    std::vector<uint8_t> bitset;
    uint64_t byte_pos;
    uint64_t pos;

protected:
    uint64_t bit_count;

public:
    obitstream();

    void write_bit(uint32_t bit);

    void flush_remaining();
};

#endif //MTF_OBITSTREAM_H
