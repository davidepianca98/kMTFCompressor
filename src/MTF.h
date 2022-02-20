
#ifndef MTF_MTF_H
#define MTF_MTF_H


#include <cstring>
#include <malloc.h>
#include <istream>
#include <iostream>
#include <vector>
#include <numeric>

class MTF {

public:

    static uint8_t moveToFront(uint8_t c, std::vector<uint8_t>& list) {
        for (int i = 0; i < list.size(); i++) {
            if (list[i] == c) {
                for (int j = i; j > 0; j--) {
                    list[j] = list[j - 1];
                }
                list[0] = c;
                return i;
            }
        }
        return c;
    }

    static void encode(std::istream& in, std::ostream& out) {
        std::vector<uint8_t> list(256);
        std::iota(std::begin(list), std::end(list), 0);

        uint8_t c;
        while (in.read(reinterpret_cast<char *>(&c), 1)) {
            uint8_t res = moveToFront(c, list);
            out.write(reinterpret_cast<const char *>(&res), 1);
        }
    }

    static uint8_t moveToFrontDecode(uint8_t i, std::vector<uint8_t>& list) {
        uint8_t ca = list[i];
        if (i != 0) {
            for (int j = i; j > 0; j--) {
                list[j] = list[j - 1];
            }
            list[0] = ca;
        }
        return ca;
    }

    static void decode(std::istream& in, std::ostream& out) {
        // Populate vector with values from 0 to 255
        std::vector<uint8_t> list(256);
        std::iota(std::begin(list), std::end(list), 0);

        uint8_t c;
        while (in.read(reinterpret_cast<char *>(&c), 1)) {
            uint8_t res = moveToFrontDecode(c, list);
            out.write(reinterpret_cast<const char *>(&res), 1);
        }
    }
};


#endif //MTF_MTF_H
