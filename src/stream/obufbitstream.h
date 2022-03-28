
#ifndef MTF_OBUFBITSTREAM_H
#define MTF_OBUFBITSTREAM_H

#include "obitstream.h"

class obufbitstream: public obitstream {
private:
    std::stringbuf sb;

public:
    explicit obufbitstream(uint8_t *buffer, int size) {
        sb.pubsetbuf(reinterpret_cast<char *>(buffer), size);
        init(&sb);
    }

    uint64_t size() {
        if (this->bit_count % 8 > 0) {
            return this->bit_count / 8 + 1;
        }
        return this->bit_count / 8;
    }
};

#endif //MTF_OBUFBITSTREAM_H
