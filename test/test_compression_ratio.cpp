
#include <filesystem>
#include <string>
#include <iostream>
#include "MTFHash.h"

int main() {
    std::string path = "../../test/resources/calgarycorpus";
    //std::string path = "../../test/resources/pizzachili";
    uint64_t size_uncompressed = 0;
    uint64_t size_compressed = 0;
    for (const auto & entry : std::filesystem::directory_iterator(path)) {
        if (entry.path().string().find(".mtf") == std::string::npos) {
            MTFHash::compress(entry.path().string(), entry.path().string() + ".mtf", 3);
            std::filesystem::path compressed(entry.path().string() + ".mtf");
            std::cout << "File name: " << entry.path() << " Uncompressed file size: " << entry.file_size()
                      << " Compressed file size: " << file_size(compressed) << " Ratio: "
                      << ((double) file_size(compressed) / (double) entry.file_size()) * 100 << std::endl;

            size_uncompressed += entry.file_size();
            size_compressed += file_size(compressed);

            std::filesystem::remove(compressed);
        }
    }
    std::cout << "Full uncompressed size: " << size_uncompressed << " Full compressed size: " << size_compressed << std::endl;

    return 0;
}

