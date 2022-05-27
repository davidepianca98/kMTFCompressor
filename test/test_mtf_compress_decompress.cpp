
#include <filesystem>
#include <string>
#include <fstream>
#include <cstring>
#include "mtf/mtftable/MTFHashTable.h"
#include "hash/randomized/RabinKarp.h"
#include "randomized/LinearHash.h"
#include "randomized/MinimiserHash.h"
#include "randomized/TabulationHash.h"


int main() {
    //std::string path = "../../test/resources/pizzachili/english";
    std::string path = "../../test/resources/calgarycorpus/obj1";
    std::ifstream in(path, std::ios::binary);
    std::ofstream out(path + ".mtfb", std::ios::binary);

    std::vector<uint8_t> data(1024 * 1024);
    std::vector<uint32_t> out_data(1024 * 1024);

    uint64_t ram = (uint64_t) 4 * 1024 * 1024 * 1024;

    std::cout << sizeof(MTFBuffer<7>) << std::endl;

    MTFHashTable<MTFBuffer<8>, 8> mtf(ram, 3, 256334);
    long read_bytes;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    do {
        // Read block
        in.read(reinterpret_cast<char *>(data.data()), 1024 * 1024);
        read_bytes = in.gcount();
        mtf.encode(data.data(), read_bytes, out_data.data());
        out.write(reinterpret_cast<char *>(out_data.data()), read_bytes * 4);
    } while (read_bytes > 0);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    in.close();
    out.close();

    mtf.print_stats();
    std::cout << "Time elapsed: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << std::endl;


    std::ifstream in2(path + ".mtfb", std::ios::binary);
    std::ofstream out2(path + ".mtf.orig", std::ios::binary);

    MTFHashTable<MTFBuffer<8>, 8> mtf2(ram, 3, 256334);
    do {
        // Read block
        in2.read(reinterpret_cast<char *>(out_data.data()), 1024 * 1024 * 4);
        read_bytes = in2.gcount();
        mtf2.decode(out_data.data(), read_bytes / 4, data.data());
        out2.write(reinterpret_cast<char *>(data.data()), read_bytes / 4);
    } while (read_bytes > 0);

    in2.close();
    out2.close();

    std::ifstream f1(path, std::ifstream::binary);
    std::ifstream f2(path + ".mtf.orig", std::ifstream::binary);

    if (!std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
                    std::istreambuf_iterator<char>(),
                    std::istreambuf_iterator<char>(f2.rdbuf()))) {
        std::cout << "ERR" << std::endl;
        return 1;
    }

    return 0;
}
