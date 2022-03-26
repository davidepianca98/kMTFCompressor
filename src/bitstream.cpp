
#include "bitstream.h"
#include <iostream>

ibitstream::ibitstream() : std::istream(nullptr), in_vec(1024 * 1024) {}

int ibitstream::readBit() {
    if (!is_open()) {
        throw std::runtime_error("ibitstream::readBit: Cannot read a bit from a stream that is not open.");
    }

    if (pos == 0 || pos >= bitset.size()) {
        read(reinterpret_cast<char *>(in_vec.data()), in_vec.size());
        long read_bytes = gcount();
        if (read_bytes <= 0) {
            return EOF;
        }

        bitset.resize(read_bytes * 8);
        boost::from_block_range(in_vec.begin(), in_vec.begin() + read_bytes, bitset);
        pos = 0;
    }

    return bitset.test(pos++);
}

bool ibitstream::remaining() {
    if (!good() && pos >= bitset.size()) {
        return false;
    }
    return true;
}

bool ibitstream::is_open() {
    return true;
}


obitstream::obitstream() : std::ostream(nullptr) {}

void obitstream::writeBit(int bit) {
    if (bit != 0 && bit != 1) {
        throw std::runtime_error(std::string("obitstream::writeBit: must pass an integer argument of 0 or 1. You passed the integer "));
    }
    if (!is_open()) {
        throw std::runtime_error("obitstream::writeBit: stream is not open");
    }

    bitset.push_back((bool) bit);

    if (bitset.size() >= 1024 * 1024 * 8) {
        std::ostream_iterator<char> osit(*this);
        boost::to_block_range(bitset, osit);
        bitset.clear();
    }
}

void obitstream::flush() {
    std::ostream_iterator<char> osit(*this);
    boost::to_block_range(bitset, osit);
}

bool obitstream::is_open() {
    return true;
}


ifbitstream::ifbitstream() {
    init(&fb);
}

ifbitstream::ifbitstream(const char* filename) {
    init(&fb);
    open(filename);
}
ifbitstream::ifbitstream(const std::string& filename) {
    init(&fb);
    open(filename);
}

void ifbitstream::open(const char* filename) {
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


ofbitstream::ofbitstream() {
    init(&fb);
}

ofbitstream::ofbitstream(const char* filename) {
    init(&fb);
    open(filename);
}

ofbitstream::ofbitstream(const std::string& filename) {
    init(&fb);
    open(filename);
}

void ofbitstream::open(const char* filename) {
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
