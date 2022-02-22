
#include <filesystem>
#include <string>
#include <iostream>
#include <fstream>
#include "MTFHash.h"

int test_file(const std::string path) { // around 20MB/s compression, 108MB/s decompression
    std::cout << path << std::endl;
    MTFHash::compress(path, path + ".mtf", 3);
    std::filesystem::path compressed(path + ".mtf");

    MTFHash::decompress(compressed.string(), compressed.string() + ".orig");

    std::ifstream f1(path, std::ifstream::binary);
    std::ifstream f2(compressed.string() + ".orig", std::ifstream::binary);

    if (!std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
                    std::istreambuf_iterator<char>(),
                    std::istreambuf_iterator<char>(f2.rdbuf()))) {
        std::cout << "ERR" << std::endl;
        return 1;
    }

    //std::filesystem::remove(compressed);
    //std::filesystem::remove(compressed.string() + ".orig");
    return 0;
}

int main() {
    std::string path = "../../test/resources/calgarycorpus";
    //std::string path = "../../test/resources/pizzachili";

    for (const auto & entry : std::filesystem::directory_iterator(path)) {
        if (entry.path().string().find(".mtf") == std::string::npos) {
            if (test_file(entry.path().string()) == 1) {
                return 1;
            }
        }
    }

    return 0;
}

