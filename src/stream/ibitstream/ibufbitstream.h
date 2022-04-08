
#ifndef MTF_IBUFBITSTREAM_H
#define MTF_IBUFBITSTREAM_H

#include <sstream>
#include "ibitstream.h"

class ibufbitstream: public ibitstream {
private:
    std::stringbuf sb;

public:
    ibufbitstream(uint8_t *buffer, int size);
};

#endif //MTF_IBUFBITSTREAM_H
