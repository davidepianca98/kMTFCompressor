
#include "obitstream.h"

obitstream::obitstream() : std::ostream(nullptr), byte_pos(0), pos(0), bit_count(0) {}

void obitstream::write_bit(uint32_t bit) {
    bitset[byte_pos] |= bit << (7 - pos);
    pos++;
    bit_count++;

    if (pos > 7) {
        byte_pos++;
        pos = 0;

        if (byte_pos >= SIZE) {
            write(reinterpret_cast<const char *>(bitset), byte_pos);
            byte_pos = 0;
        }
        bitset[byte_pos] = 0;
    }
}

void obitstream::flush_remaining() {
    if (pos > 0) {
        byte_pos++;
    }
    write(reinterpret_cast<const char *>(bitset), byte_pos);
    pos = 0;
    byte_pos = 0;
    flush();
}
