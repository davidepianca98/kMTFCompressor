
#include <cstdio>
#include "ibitstream.h"

ibitstream::ibitstream() : std::istream(nullptr), bitset(1024 * 1024), byte_pos(0), pos(0) {}

int ibitstream::read_bit() {
    if (byte_pos == 0 && pos == 0) {
        read(reinterpret_cast<char *>(bitset.data()), (long) bitset.size());
        long read_bytes = gcount();
        if (read_bytes <= 0) {
            return EOF;
        }

        bitset.resize(read_bytes);
        byte_pos = 0;
        pos = 0;
    }

    int val = (bitset[byte_pos] >> (7 - pos)) & 1;
    pos++;
    if (pos > 7) {
        byte_pos++;
        pos = 0;

        if (byte_pos >= bitset.size()) {
            byte_pos = 0;
        }
    }

    return val;
}

bool ibitstream::remaining() {
    if (!good() && byte_pos >= bitset.size()) {
        return false;
    }
    return true;
}
