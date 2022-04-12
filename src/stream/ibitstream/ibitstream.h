
#ifndef MTF_IBITSTREAM_H
#define MTF_IBITSTREAM_H

#include <istream>
#include <vector>

class ibitstream: public std::istream {
private:
    static constexpr int SIZE = 4 * 1024 * 1024;

    uint8_t bitset[SIZE] = { 0 };
    uint64_t byte_pos;
    uint64_t pos;
    uint64_t read_bytes;

public:
    ibitstream();

    int read_bit();

    bool remaining();
};

#endif //MTF_IBITSTREAM_H
