
#ifndef MTF_MTF_H
#define MTF_MTF_H

#include <cstdint>
#include <fstream>
#include <vector>

class MTF {

public:

    static uint8_t move_to_front(uint8_t c, std::vector<uint8_t>& list);

    static void encode(std::istream& in, std::ostream& out);

    static int compress(const std::string& path, const std::string& out_path);

    static uint8_t move_to_front_decode(uint8_t i, std::vector<uint8_t>& list);

    static void decode(std::istream& in, std::ostream& out);
};


#endif //MTF_MTF_H
