
#ifndef MTF_OBITSTREAM_H
#define MTF_OBITSTREAM_H

#include <ostream>
#include <vector>

class obitstream: public std::ostream {
private:
    std::vector<uint8_t> bitset;
    int byte_pos;
    int pos;

protected:
    uint64_t bit_count;

public:
    obitstream();

    void write_bit(int bit);

    void flush_remaining();
};

#endif //MTF_OBITSTREAM_H
