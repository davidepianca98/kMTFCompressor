
#ifndef MTF_IBITSTREAM_H
#define MTF_IBITSTREAM_H

#include <istream>
#include <vector>

class ibitstream: public std::istream {
private:
    std::vector<uint8_t> bitset;
    uint64_t byte_pos;
    uint64_t pos;

public:
    ibitstream();

    int read_bit();

    bool remaining();
};

#endif //MTF_IBITSTREAM_H
