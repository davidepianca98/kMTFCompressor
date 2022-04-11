
#include <string>
#include "mtf/MTFHashCompressor.h"
#include "randomized/RabinKarp.h"


int main(int argc, char *argv[]) {
    if (argc != 3) {
        return 1;
    }

    std::string path = argv[1];
    int k = (int) strtol(argv[2], nullptr, 10);

    std::cout << "Compressing\n";
    MTFHashCompressor::compress_stream<RabinKarp, 8>(path, path + ".mtf", k, (uint64_t) 4 * 1024 * 1024 * 1024);
    std::cout << "Decompressing\n";
    MTFHashCompressor::decompress_stream<RabinKarp, 8>(path + ".mtf", path + ".mtf" + ".orig", k, (uint64_t) 4 * 1024 * 1024 * 1024);

    //MTFHashCompressor::compress_block<RabinKarp, 8>(path, path + ".mtf", k, (uint64_t) 4 * 1024 * 1024 * 1024);

    return 0;
}
