
#ifndef MTF_OBUFBITSTREAM_H
#define MTF_OBUFBITSTREAM_H

#include <sstream>
#include "obitstream.h"

class obufbitstream: public obitstream {
private:
    std::stringbuf sb;

public:
    obufbitstream(uint8_t *buffer, int size);

    uint64_t size();
};

#endif //MTF_OBUFBITSTREAM_H
