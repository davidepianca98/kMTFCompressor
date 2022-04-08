
#ifndef MTF_OFBITSTREAM_H
#define MTF_OFBITSTREAM_H

#include <fstream>
#include "obitstream.h"

class ofbitstream: public obitstream {
private:
    std::filebuf fb;

public:
    explicit ofbitstream(const char *filename);

    explicit ofbitstream(const std::string& filename);

    void open(const char *filename);

    void open(const std::string& filename);

    bool is_open();

    void close();
};

#endif //MTF_OFBITSTREAM_H
