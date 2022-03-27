/*
 * File: bitstream.h
 * -----------------
 * This file defines the ibitstream and obitstream classes which are basically
 * same as the ordinary istream and ostream classes, but add the
 * functionality to read and write one bit at a time.
 *
 * The idea is that you can substitute an ibitstream in place of an
 * istream and use the same operations (get, fail, >>, etc.)
 * along with added member functions of readBit, rewind, and size.
 *
 * Similarly, the obitstream can be used in place of ofstream, and has
 * same operations (put, fail, <<, etc.) along with additional
 * member functions writeBit and size.
 *
 * There are two subclasses of ibitstream: ifbitstream and istringbitstream,
 * which are similar to the ifstream and istringstream classes.  The
 * obitstream class similarly has ofbitstream and ostringbitstream as
 * subclasses.
 *
 * @author Keith Schwarz, Eric Roberts, Marty Stepp
 * @version 2016/11/12
 * - made toPrintable non-static and visible
 */

#ifndef _bitstream_h
#define _bitstream_h

#include <istream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <bitset>
#include <boost/dynamic_bitset.hpp>


class ibitstream: public std::istream {
public:
    ibitstream();

    int readBit();

    virtual bool is_open();

    bool remaining();

private:
    boost::dynamic_bitset<unsigned char> bitset;
    std::vector<uint8_t> in_vec;
    int pos = 0;
};


class obitstream: public std::ostream {
public:
    obitstream();

    void writeBit(int bit);

    void flush();

private:
    boost::dynamic_bitset<unsigned char> bitset;
    int pos;
};


class ifbitstream: public ibitstream {
public:
    ifbitstream();

    explicit ifbitstream(const char* filename);
    explicit ifbitstream(const std::string& filename);

    void open(const char* filename);
    void open(const std::string& filename);

    bool is_open();

    void close();

private:
    std::filebuf fb;
};


class ofbitstream: public obitstream {
public:
    ofbitstream();

    explicit ofbitstream(const char* filename);
    explicit ofbitstream(const std::string& filename);

    void open(const char* filename);
    void open(const std::string& filename);

    bool is_open();

    void close();

private:
    std::filebuf fb;
};

#endif
