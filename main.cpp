
#include <algorithm>
#include <string>
#include "mtf/MTFHashCompressor.h"
#include "randomized/TabulationHash.h"

char *get_option(char **begin, char **end, const std::string& option) {
    char **itr = std::find(begin, end, option);
    if (itr != end && ++itr != end) {
        return *itr;
    }
    return nullptr;
}

bool has_option(char **begin, char **end, const std::string& option) {
    return std::find(begin, end, option) != end;
}

int main(int argc, char *argv[]) {
    char *path_str = get_option(argv, argv + argc, "-f");
    if (!path_str) {
        throw std::runtime_error("File name not provided");
    }
    std::string path = std::string(path_str);

    bool compress = has_option(argv, argv + argc, "-c");
    bool decompress = has_option(argv, argv + argc, "-d");

    bool multithreaded = has_option(argv, argv + argc, "-p");

    char *k_str = get_option(argv, argv + argc, "-k");
    int k = 3;
    if (k_str) {
        k = (int) strtol(k_str, nullptr, 10);
    }

    char *mem_str = get_option(argv, argv + argc, "-m");
    uint64_t max_mem = (uint64_t) 4 * 1024 * 1024 * 1024;
    if (mem_str) {
        max_mem = strtoull(k_str, nullptr, 10);
    }

    std::string suffix = ".mtf";
    if (compress && decompress) {
        throw std::runtime_error("Both compression and decompression selected");
    } else if (compress) {
        std::cout << "Compressing\n";
        if (!multithreaded) {
            MTFHashCompressor::compress_stream<TabulationHash, 8>(path, path + suffix, k, max_mem);
        } else {
            MTFHashCompressor::compress_block<TabulationHash, 8>(path, path + suffix, k, max_mem);
        }
    } else if (decompress) {
        std::cout << "Decompressing\n";
        std::string original_name = path.substr(0, path.length() - suffix.length());
        if (!multithreaded) {
            MTFHashCompressor::decompress_stream<TabulationHash, 8>(path, original_name, k, max_mem);
        } else {
            MTFHashCompressor::decompress_block<TabulationHash, 8>(path, original_name, k, max_mem);
        }
    } else {
        throw std::runtime_error("Neither compression nor decompression selected");
    }

    return 0;
}
