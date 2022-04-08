
#ifndef MTF_IFBITSTREAM_H
#define MTF_IFBITSTREAM_H

#include <fstream>
#include "ibitstream.h"

class ifbitstream: public ibitstream {
private:
    std::filebuf fb;

public:
    ifbitstream();

    explicit ifbitstream(const char *filename);

    explicit ifbitstream(const std::string& filename);

    void open(const char *filename);

    void open(const std::string& filename);

    bool is_open();

    void close();
};

#endif //MTF_IFBITSTREAM_H
