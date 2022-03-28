
#ifndef MTF_IFBITSTREAM_H
#define MTF_IFBITSTREAM_H

#include <fstream>
#include "ibitstream.h"

class ifbitstream: public ibitstream {
private:
    std::filebuf fb;

public:
    ifbitstream() {
        init(&fb);
    }

    explicit ifbitstream(const char* filename) {
        init(&fb);
        open(filename);
    }

    explicit ifbitstream(const std::string& filename) {
        init(&fb);
        open(filename);
    }

    void open(const char *filename) {
        if (!fb.open(filename, std::ios::in | std::ios::binary)) {
            setstate(std::ios::failbit);
        }
    }

    void open(const std::string& filename) {
        open(filename.c_str());
    }

    bool is_open() {
        return fb.is_open();
    }

    void close() {
        if (!fb.close()) {
            setstate(std::ios::failbit);
        }
    }
};

#endif //MTF_IFBITSTREAM_H
