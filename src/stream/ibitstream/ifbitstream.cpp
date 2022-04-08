
#include <string>
#include "ifbitstream.h"

ifbitstream::ifbitstream() {
    init(&fb);
}

ifbitstream::ifbitstream(const char *filename) {
    init(&fb);
    open(filename);
}

ifbitstream::ifbitstream(const std::string& filename) {
    init(&fb);
    open(filename);
}

void ifbitstream::open(const char *filename) {
    if (!fb.open(filename, std::ios::in | std::ios::binary)) {
        setstate(std::ios::failbit);
    }
}

void ifbitstream::open(const std::string& filename) {
    open(filename.c_str());
}

bool ifbitstream::is_open() {
    return fb.is_open();
}

void ifbitstream::close() {
    if (!fb.close()) {
        setstate(std::ios::failbit);
    }
}
