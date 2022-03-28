
#include <string>
#include <fstream>
#include <sstream>

#include "MTFHashCompressor.h"
#include "MTF.h"
#include "MTFBlockWorker.h"


int main(int argc, char *argv[]) {
    if (argc != 3) {
        return 1;
    }

    std::string path = argv[1];
    int k = (int) strtol(argv[2], nullptr, 10);

    MTFHashCompressor::compress_stream(path, path + ".mtf", k);

    return 0;
}
