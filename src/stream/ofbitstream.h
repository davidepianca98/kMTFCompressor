
#ifndef MTF_OFBITSTREAM_H
#define MTF_OFBITSTREAM_H

#include <fstream>
#include "obitstream.h"

class ofbitstream: public obitstream {
private:
    std::filebuf fb;

public:
    explicit ofbitstream(const char *filename) {
        init(&fb);
        open(filename);
    }

    explicit ofbitstream(const std::string& filename) {
        init(&fb);
        open(filename);
    }

    void open(const char *filename) {
        if (!fb.open(filename, std::ios::out | std::ios::binary)) {
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

#endif //MTF_OFBITSTREAM_H
