
#include "ofbitstream.h"

ofbitstream::ofbitstream(const char *filename) {
    init(&fb);
    open(filename);
}

ofbitstream::ofbitstream(const std::string& filename) {
    init(&fb);
    open(filename);
}

void ofbitstream::open(const char *filename) {
    if (!fb.open(filename, std::ios::out | std::ios::binary)) {
        setstate(std::ios::failbit);
    }
}

void ofbitstream::open(const std::string& filename) {
    open(filename.c_str());
}

bool ofbitstream::is_open() {
    return fb.is_open();
}

void ofbitstream::close() {
    if (!fb.close()) {
        setstate(std::ios::failbit);
    }
}
