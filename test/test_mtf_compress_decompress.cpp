
#include <filesystem>
#include <string>
#include <fstream>
#include <cstring>
#include "MTFHashTable.h"

int main() {
    std::ifstream ifs("../../test/resources/calgarycorpus/bib", std::ios::binary);
    //std::ifstream ifs("../../test/resources/pizzachili/dblp.xml", std::ios::binary);

    ifs.seekg(0, std::ios::end);
    size_t file_size = ifs.tellg();
    std::vector<char> data;
    data.resize(file_size);
    ifs.seekg(0, std::ios::beg);
    ifs.read(&data[0], file_size);

    ifs.close();

    std::vector<uint32_t> out_data(file_size);

    MTFHashTable mtf(3);
    mtf.encode(reinterpret_cast<const uint8_t *>(data.data()), file_size, out_data.data());

    for (auto c: out_data) {
        std::cout << c << " ";
    }

    MTFHashTable mtf2(3);
    mtf2.decode(out_data.data(), file_size, reinterpret_cast<uint8_t *>(out_data.data()));

    if (memcmp(data.data(), out_data.data(), file_size) != 0) {
        return 1;
    }

    return 0;
}
