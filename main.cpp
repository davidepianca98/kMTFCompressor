
#include <string>
#include "MTFHashCompressor.h"


int main(int argc, char *argv[]) {
    if (argc != 3) {
        return 1;
    }

    std::string path = argv[1];
    int k = (int) strtol(argv[2], nullptr, 10);

    MTFHashCompressor::compress_stream<RabinKarp, uint64_t>(path, path + ".mtf", k);

    return 0;
}
