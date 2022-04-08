
#ifndef MTF_IBITSTREAM_H
#define MTF_IBITSTREAM_H

#include <istream>
#include <vector>

class ibitstream: public std::istream {
private:
    std::vector<uint8_t> bitset;
    int byte_pos;
    int pos;

public:
    ibitstream();

    int read_bit();

    bool remaining();
};

#endif //MTF_IBITSTREAM_H
