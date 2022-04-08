
#include <filesystem>
#include <string>
#include <iostream>
#include "mtf/MTFHashCompressor.h"
#include "mtf/MTF.h"

int main() {
    //std::string path = "../../test/resources/calgarycorpus";
    //std::string path = "../../test/resources/canterbury";
    std::string path = "../../test/resources/pizzachili";
    //std::string path = "../../test/resources/mio";
    uint64_t size_uncompressed = 0;
    uint64_t size_compressed = 0;
    for (const auto & entry : std::filesystem::directory_iterator(path)) {
        if (entry.path().string().find(".mtf") == std::string::npos) {
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            MTF::compress(entry.path().string(), entry.path().string() + ".mtfc");
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

            std::filesystem::path compressed(entry.path().string() + ".mtfc");
            std::cout << "File name: " << entry.path() << ", Uncompressed file size: " << entry.file_size()
                      << ", Compressed file size: " << file_size(compressed) << ", Ratio: "
                      << ((double) file_size(compressed) / (double) entry.file_size()) * 100 << ", Time elapsed: "
                      << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl << std::endl;

            size_uncompressed += entry.file_size();
            size_compressed += file_size(compressed);

            //std::filesystem::remove(compressed);
        }
    }
    std::cout << "Full uncompressed size: " << size_uncompressed << std::endl;
    std::cout << "Full compressed size: " << size_compressed << std::endl;
    std::cout << "Compression ratio: " << (((double) size_compressed) / ((double) size_uncompressed)) * 100 << std::endl;

    return 0;
}
