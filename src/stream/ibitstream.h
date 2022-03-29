
#ifndef MTF_IBITSTREAM_H
#define MTF_IBITSTREAM_H

class ibitstream: public std::istream {
private:
    std::vector<uint8_t> bitset;
    int byte_pos;
    int pos;

public:
    ibitstream() : std::istream(nullptr), bitset(1024 * 1024), byte_pos(0), pos(0) {}

    int readBit() {
        if (byte_pos == 0 && pos == 0) {
            read(reinterpret_cast<char *>(bitset.data()), bitset.size());
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

    bool remaining() {
        if (!good() && byte_pos >= bitset.size()) {
            return false;
        }
        return true;
    }
};

#endif //MTF_IBITSTREAM_H
