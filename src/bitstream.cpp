/*
 * File: bitstream.cpp
 * -------------------
 * This file contains the implementation of ibitstream and obitstream classes.
 * These classes are patterned after (and, in fact, inherit from) the standard
 * ifstream and ofstream classes.  Please see bitstream.h for information about
 * how a client properly uses these classes.
 *
 * @author Keith Schwarz, Eric Roberts, Marty Stepp
 * @version 2016/11/12
 * - made toPrintable non-static and visible
 * @version 2014/10/08
 * - removed 'using namespace' statement
 * 2014/01/23
 * - added slightly more descriptive error messages e.g. in writeBit
 * - whitespace reformatting
 * Previously last modified on Mon May 21 19:50:00 PST 2012 by Keith Schwarz
 */

#include "bitstream.h"
#include <iostream>

/* Constructor ibitstream::ibitstream
 * ----------------------------------
 * Each ibitstream tracks 3 integers as private data.
 * "lastTell" is streampos of the last byte that was read (this is used
 * to detect when other non-readBit activity has changed the tell)
 * "curByte" contains contents of byte currently being read
 * "pos" is the bit position within curByte that is next to read
 * We set initial state for lastTell and curByte to 0, then pos is
 * set at 8 so that next readBit will trigger a fresh read.
 */
ibitstream::ibitstream() : std::istream(nullptr), in_vec(1024 * 1024) {}

/* Member function ibitstream::readBit
 * -----------------------------------
 * If bits remain in curByte, retrieve next and increment pos
 * Else if end of curByte (or some other read happened), then read next byte
 * and start reading from bit position 0 of that byte.
 * If read byte from file at EOF, return EOF.
 */
int ibitstream::readBit() {
    if (!is_open()) {
        throw std::runtime_error("ibitstream::readBit: Cannot read a bit from a stream that is not open.");
    }

    if (pos == 0 || pos >= bitset.size()) {
        read(reinterpret_cast<char *>(in_vec.data()), in_vec.size());
        long read_bytes_amount = gcount();
        if (read_bytes_amount <= 0) {
            return EOF;
        }

        bitset.resize(read_bytes_amount * 8);
        boost::from_block_range(in_vec.begin(), in_vec.begin() + read_bytes_amount, bitset);
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

/* Member function ibitstream::is_open
 * -----------------------------------
 * Default implementation of is_open has the stream always
 * open.  Subclasses can customize this if they'd like.
 */
bool ibitstream::is_open() {
    return true;
}

/* Constructor obitstream::obitstream
 * ----------------------------------
 * Each obitstream tracks 3 integers as private data.
 * "lastTell" is streampos of the last byte that was written (this is used
 * to detect when other non-writeBit activity has changed the tell)
 * "curByte" contains contents of byte currently being written
 * "pos" is the bit position within curByte that is next to write
 * We set initial state for lastTell and curByte to 0, then pos is
 * set at 8 so that next writeBit will start a new byte.
 */
obitstream::obitstream() : std::ostream(nullptr) {}

/* Member function obitstream::writeBit
 * ------------------------------------
 * If bits remain to be written in curByte, add bit into byte and increment pos
 * Else if end of curByte (or some other write happened), then start a fresh
 * byte at position 0.
 * We write the byte out for each bit (backing up to overwrite as needed), rather
 * than waiting for 8 bits.  This is because the client might make
 * 3 writeBit calls and then start using << so we can't wait til full-byte
 * boundary to flush any partial-byte bits.
 */
void obitstream::writeBit(int bit) {
    if (bit != 0 && bit != 1) {
        throw std::runtime_error(std::string("obitstream::writeBit: must pass an integer argument of 0 or 1. You passed the integer "));
    }
    if (!is_open()) {
        throw std::runtime_error("obitstream::writeBit: stream is not open");
    }

    bitset.push_back(bit);

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

/* Member function obitstream::is_open
 * -----------------------------------
 * Default implementation of is_open has the stream always
 * open.  Subclasses can customize this if they'd like.
 */
bool obitstream::is_open() {
    return true;
}

/* Constructor ifbitstream::ifbitstream
 * ------------------------------------
 * Wires up the stream class so that it knows to read data
 * from disk.
 */
ifbitstream::ifbitstream() {
    init(&fb);
}

/* Constructor ifbitstream::ifbitstream
 * ------------------------------------
 * Wires up the stream class so that it knows to read data
 * from disk, then opens the given file.
 */
ifbitstream::ifbitstream(const char* filename) {
    init(&fb);
    open(filename);
}
ifbitstream::ifbitstream(const std::string& filename) {
    init(&fb);
    open(filename);
}

/* Member function ifbitstream::open
 * ---------------------------------
 * Attempts to open the specified file, failing if unable
 * to do so.
 */
void ifbitstream::open(const char* filename) {
    if (!fb.open(filename, std::ios::in | std::ios::binary)) {
        setstate(std::ios::failbit);
    }
}

void ifbitstream::open(const std::string& filename) {
    open(filename.c_str());
}

/* Member function ifbitstream::is_open
 * ------------------------------------
 * Determines whether the file stream is open.
 */
bool ifbitstream::is_open() {
    return fb.is_open();
}

/* Member function ifbitstream::close
 * ----------------------------------
 * Closes the file stream, if one is open.
 */
void ifbitstream::close() {
    if (!fb.close()) {
        setstate(std::ios::failbit);
    }
}

/* Constructor ofbitstream::ofbitstream
 * ------------------------------------
 * Wires up the stream class so that it knows to write data
 * to disk.
 */
ofbitstream::ofbitstream() {
    init(&fb);
}

/* Constructor ofbitstream::ofbitstream
 * ------------------------------------
 * Wires up the stream class so that it knows to write data
 * to disk, then opens the given file.
 */
ofbitstream::ofbitstream(const char* filename) {
    init(&fb);
    open(filename);
}

ofbitstream::ofbitstream(const std::string& filename) {
    init(&fb);
    open(filename);
}

/* Member function ofbitstream::open
 * ---------------------------------
 * Attempts to open the specified file, failing if unable
 * to do so.
 */
void ofbitstream::open(const char* filename) {
    if (!fb.open(filename, std::ios::out | std::ios::binary)) {
        setstate(std::ios::failbit);
    }
}
void ofbitstream::open(const std::string& filename) {
    open(filename.c_str());
}

/* Member function ofbitstream::is_open
 * ------------------------------------
 * Determines whether the file stream is open.
 */
bool ofbitstream::is_open() {
    return fb.is_open();
}

/* Member function ofbitstream::close
 * ----------------------------------
 * Closes the given file.
 */
void ofbitstream::close() {
    if (!fb.close()) {
        setstate(std::ios::failbit);
    }
}
