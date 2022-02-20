
#ifndef MTF_RLE_H
#define MTF_RLE_H


#include <istream>
#include <iostream>

class RLE {
public:
    static void encode(std::istream& in, std::ostream& out) {
        char c;
        while (in.get(c)) {
            uint8_t count = 1; // TODO if more than 255 we need to split
            char d;
            while (in.get(d) && c == d) {
                count++;
            }
            out << c << count;
            in.putback(d);
        }
    }

    static void decode(std::istream& in, std::ostream& out) {
        char c;
        char count;
        while (in.get(c) && in.get(count)) {
            for (int i = 0; i < count; i++) {
                out << c;
            }
        }
    }
};


#endif //MTF_RLE_H
