
#ifndef MTF_IBITSTREAM_H
#define MTF_IBITSTREAM_H

#include <boost/dynamic_bitset.hpp>

class ibitstream: public std::istream {
private:
    boost::dynamic_bitset<unsigned char> bitset;
    std::vector<uint8_t> in_vec;
    int pos = 0;

public:
    ibitstream() : std::istream(nullptr), in_vec(1024 * 1024) {}

    int readBit() {
        if (pos == 0 || pos >= bitset.size()) {
            read(reinterpret_cast<char *>(in_vec.data()), in_vec.size());
            long read_bytes = gcount();
            if (read_bytes <= 0) {
                return EOF;
            }

            bitset.resize(read_bytes * 8);
            boost::from_block_range(in_vec.begin(), in_vec.begin() + read_bytes, bitset);
            pos = 0;
        }

        return bitset.test(pos++);
    }

    bool remaining() {
        if (!good() && pos >= bitset.size()) {
            return false;
        }
        return true;
    }
};

#endif //MTF_IBITSTREAM_H
