
#include <filesystem>
#include <string>
#include <iostream>
#include "MTFHashCompressor.h"
//#include "SequenceCompressor.h"
//#include "MTF.h"

int main() {
    //std::string path = "../../test/resources/calgarycorpus";
    //std::string path = "../../test/resources/canterbury";
    //std::string path = "../../test/resources/pizzachili";
    //std::string path = "../../test/resources/pizzachilirep";
    std::string path = "../../test/resources/mio";
    //std::string path = "../../test/resources/maximumcompression";
    uint64_t size_uncompressed = 0;
    uint64_t size_compressed = 0;
    uint64_t total_time = 0;
    for (const auto & entry : std::filesystem::directory_iterator(path)) {
        if (entry.path().string().find(".mtf") == std::string::npos) {
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

            std::cout << "File name: " << entry.path() << ", Uncompressed file size: " << entry.file_size() << std::endl;

            MTFHashCompressor::compress_block(entry.path().string(), entry.path().string() + ".mtf", 3);
            //MTFHashCompressor::compress_stream(entry.path().string(), entry.path().string() + ".mtf", 3);
            //MTF::compress(entry.path().string(), entry.path().string() + ".mtf");
            //SequenceCompressor::compress_stream(entry.path().string(), entry.path().string() + ".mtf");
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

            std::filesystem::path compressed(entry.path().string() + ".mtf");

            uint64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            total_time += time;

            double ratio = ((double) file_size(compressed) / (double) entry.file_size()) * 100;

            std::cout << "Compressed file size: " << file_size(compressed) << ", Ratio: " << ratio
                        << "%, Time elapsed: " << time << " ms" << std::endl << std::endl;

            size_uncompressed += entry.file_size();
            size_compressed += file_size(compressed);

            //std::filesystem::remove(compressed);
        }
    }
    std::cout << "Full uncompressed size: " << size_uncompressed << std::endl;
    std::cout << "Full compressed size: " << size_compressed << std::endl;
    std::cout << "Total elapsed time: " << total_time << std::endl;
    std::cout << "Total compression ratio: " << (((double) size_compressed) / ((double) size_uncompressed)) * 100 << std::endl;

    return 0;
}

