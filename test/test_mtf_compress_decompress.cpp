
#include <filesystem>
#include <string>
#include <fstream>
#include <cstring>
#include "MTFHashTable.h"
#include "MTFHashTableBlock.h"
#include "RabinKarp.h"


int main() {
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

    RabinKarp hash(3, 4096);
    MTFHashTableBlock<uint64_t> mtf(1024 * 1024, hash);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    mtf.encode(reinterpret_cast<const uint8_t *>(data.data()), file_size, out_data.data());
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    for (auto c: out_data) {
        //std::cout << c << " ";
    }

    mtf.print_stats();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;

    RabinKarp hash2(3, 4096);
    MTFHashTableBlock<uint64_t> mtf2(1024 * 1024, hash2);
    mtf2.decode(out_data.data(), file_size, reinterpret_cast<uint8_t *>(out_data.data()));

    if (memcmp(data.data(), out_data.data(), file_size) != 0) {
        return 1;
    }

    return 0;
}
