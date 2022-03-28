
#ifndef MTF_OBITSTREAM_H
#define MTF_OBITSTREAM_H

#include <boost/dynamic_bitset.hpp>

class obitstream: public std::ostream {
private:
    boost::dynamic_bitset<unsigned char> bitset;
    int pos;

protected:
    uint64_t bit_count;

public:
    obitstream() : std::ostream(nullptr), bitset(1024 * 1024 * 8), pos(0), bit_count(0) {}

    void writeBit(int bit) {
        bitset[pos++] = (bool) bit;
        bit_count++;

        if (pos >= bitset.size()) {
            std::ostream_iterator<char> osit(*this);
            boost::to_block_range(bitset, osit);
            pos = 0;
        }
    }

    void flush_remaining() {
        bitset.resize(pos);
        std::ostream_iterator<char> osit(*this);
        boost::to_block_range(bitset, osit);
        pos = 0;
        flush();
    }
};

#endif //MTF_OBITSTREAM_H
