
#ifndef MTF_MTF_H
#define MTF_MTF_H


#include <cstring>
#include <malloc.h>
#include <istream>
#include <iostream>
#include <vector>
#include <numeric>
#include <fstream>
#include "MTFBlockWorker.h"

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

    static int compress(const std::string& path, const std::string& out_path) {
        std::ifstream in_file(path, std::ios::binary);
        if (in_file.fail()) {
            return 1;
        }
        ofbitstream out_file(out_path);

        int block_size = 1024 * 1024; // 1 MB block size
        auto *in_data = new uint8_t[block_size];

        AdaptiveHuffman ah(256 + 1);

        std::vector<uint8_t> list(256);
        std::iota(std::begin(list), std::end(list), 0);

        while (in_file.good()) {
            in_file.read(reinterpret_cast<char *>(in_data), block_size);
            long read_bytes = in_file.gcount();

            for (int i = 0; i < read_bytes; i++) {
                ah.encode(moveToFront(in_data[i], list), out_file);
            }
        }
        ah.encode(255, out_file); // EOF

        out_file.flush_remaining();

        delete[] in_data;

        in_file.close();
        out_file.close();

        return 0;
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
