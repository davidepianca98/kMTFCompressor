
#ifndef MTF_IBUFBITSTREAM_H
#define MTF_IBUFBITSTREAM_H

#include "ibitstream.h"

class ibufbitstream: public ibitstream {
private:
    std::stringbuf sb;

public:
    explicit ibufbitstream(uint8_t *buffer, int size) {
        sb.pubsetbuf(reinterpret_cast<char *>(buffer), size);
        init(&sb);
    }
};

#endif //MTF_IBUFBITSTREAM_H
