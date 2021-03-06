
#include <filesystem>
#include <string>
#include <iostream>
#include <fstream>
#include "mtf/MTFHashCompressor.h"

int test_file(const std::string& path) {
    uint64_t ram = (uint64_t) 4 * 1024 * 1024 * 1024;

    std::cout << "Compressing: " << path << std::endl;
    MTFHashCompressor::compress_stream<MTFBuffer<8>, 8>(path, path + ".mtf", 3, ram);
    //MTFHashCompressor::compress_block<MTFBuffer<8>, 8>(path, path + ".mtf", 3, ram);
    std::filesystem::path compressed(path + ".mtf");

    std::cout << "Decompressing\n";

    MTFHashCompressor::decompress_stream<MTFBuffer<8>, 8>(compressed.string(), compressed.string() + ".orig", 3, ram);
    //MTFHashCompressor::decompress_block<MTFBuffer<8>, 8>(compressed.string(), compressed.string() + ".orig", 3, ram);

    std::cout << "Checking\n";

    std::ifstream f1(path, std::ifstream::binary);
    std::ifstream f2(compressed.string() + ".orig", std::ifstream::binary);

    if (!std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
                    std::istreambuf_iterator<char>(),
                    std::istreambuf_iterator<char>(f2.rdbuf()))) {
        std::cout << "ERR" << std::endl;
        return 1;
    }

    std::cout << "OK\n";

    //std::filesystem::remove(compressed);
    //std::filesystem::remove(compressed.string() + ".orig");
    return 0;
}

int main() {
    std::string path = "../../test/resources/calgarycorpus";
    //std::string path = "../../test/resources/mio";
    //std::string path = "../../test/resources/pizzachilirep";
    //std::string path = "../../test/resources/pizzachili";
    //std::string path = "../../test/resources/pizzachilismall";

    for (const auto & entry : std::filesystem::directory_iterator(path)) {
        if (entry.path().string().find(".mtf") == std::string::npos) {
            if (test_file(entry.path().string()) == 1) {
                return 1;
            }
        }
    }

    return 0;
}

