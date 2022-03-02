
#include <filesystem>
#include <string>
#include <fstream>
#include <cstring>
#include "MTFHashTable.h"


void test() {
    std::ifstream ifs("../../test/resources/calgarycorpus/bib", std::ios::binary);
    MTFHashTable mtf(3);
    std::ostringstream oss;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    mtf.encode2(ifs, oss);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    ifs.close();
    for (auto c: oss.str()) {
        //std::cout << c << " ";
    }
    mtf.print_stats();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;
}

void test2() {
    std::ifstream ifs("../../test/resources/calgarycorpus/bib", std::ios::binary);
    //std::ifstream ifs("../../test/resources/pizzachili/pitches", std::ios::binary);
    ifs.seekg(0, std::ios::end);
    size_t file_size = ifs.tellg();
    std::vector<char> data;
    data.resize(file_size);
    ifs.seekg(0, std::ios::beg);
    ifs.read(&data[0], file_size);

    ifs.close();

    std::vector<uint32_t> out_data(file_size);

    MTFHashTable mtf(3);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    mtf.encode3(reinterpret_cast<const uint8_t *>(data.data()), file_size, out_data.data());
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    ifs.close();
    for (auto c: out_data) {
        std::cout << c << " ";
    }
    mtf.print_stats();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;




    /*std::vector<uint32_t> out_data2(file_size);
    MTFHashTable mtf2(3);
    mtf2.encode(reinterpret_cast<const uint8_t *>(data.data()), file_size, out_data2.data());

    if (memcmp(out_data.data(), out_data2.data(), file_size) != 0) {
        std::cout << "ERR" << std::endl;
    }*/
}

int main() {
    //test();
    //test2();

    std::ifstream ifs("../../test/resources/calgarycorpus/bib", std::ios::binary);
    //std::ifstream ifs("../../test/resources/pizzachili/pitches", std::ios::binary); // TODO timer

    ifs.seekg(0, std::ios::end);
    size_t file_size = ifs.tellg();
    std::vector<char> data;
    data.resize(file_size);
    ifs.seekg(0, std::ios::beg);
    ifs.read(&data[0], file_size);

    ifs.close();

    std::vector<uint32_t> out_data(file_size);

    MTFHashTable mtf(3);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    mtf.encode3(reinterpret_cast<const uint8_t *>(data.data()), file_size, out_data.data());
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    for (auto c: out_data) {
        //std::cout << c << " ";
    }

    mtf.print_stats();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;

    MTFHashTable mtf2(3);
    mtf2.decode(out_data.data(), file_size, reinterpret_cast<uint8_t *>(out_data.data()));

    if (memcmp(data.data(), out_data.data(), file_size) != 0) {
        return 1;
    }

    return 0;
}
