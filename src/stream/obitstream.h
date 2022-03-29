
#ifndef MTF_OBITSTREAM_H
#define MTF_OBITSTREAM_H

class obitstream: public std::ostream {
private:
    std::vector<uint8_t> bitset;
    int byte_pos;
    int pos;

protected:
    uint64_t bit_count;

public:
    obitstream() : std::ostream(nullptr), bitset(1024 * 1024, 0), byte_pos(0), pos(0), bit_count(0) {}

    void writeBit(int bit) {
        bitset[byte_pos] |= bit << (7 - pos);
        pos++;
        bit_count++;

        if (pos > 7) {
            byte_pos++;
            pos = 0;

            if (byte_pos >= bitset.size()) {
                write(reinterpret_cast<const char *>(bitset.data()), byte_pos);
                byte_pos = 0;
            }
            bitset[byte_pos] = 0;
        }
    }

    void flush_remaining() {
        if (pos > 0) {
            byte_pos++;
        }
        write(reinterpret_cast<const char *>(bitset.data()), byte_pos);
        pos = 0;
        byte_pos = 0;
        flush();
    }
};

#endif //MTF_OBITSTREAM_H
