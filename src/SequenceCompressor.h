
#ifndef MTF_SEQUENCECOMPRESSOR_H
#define MTF_SEQUENCECOMPRESSOR_H

#include <istream>
#include <fstream>
#include "encoders/FastPForEncoder.h"

class SequenceCompressor {
public:

    static void encode(std::istream& in, std::ostream& out, int block_size) {
        auto *in_data = new uint8_t[block_size];
        auto *in_data2 = new uint32_t[block_size];
        auto *out_data = new uint32_t[block_size + 1024];

        long read_bytes;
        do {
            // Read block
            in.read(reinterpret_cast<char *>(in_data), block_size);
            read_bytes = in.gcount();

            if (read_bytes > 0) {
                for (int i = 0; i < read_bytes; i++) {
                    in_data2[i] = in_data[i];
                }

                uint32_t compressed_size = FastPForEncoder::compress(in_data2, read_bytes, out_data);

                out.write(reinterpret_cast<const char *>(&compressed_size), 4);
                out.write(reinterpret_cast<const char *>(out_data), (long) compressed_size);
            }
        } while (read_bytes > 0);

        delete[] in_data;
        delete[] in_data2;
        delete[] out_data;
    }

    static int compress_stream(const std::string& path, const std::string& out_path) {
        std::ifstream in_file(path, std::ios::binary);
        if (in_file.fail()) {
            return 1;
        }
        std::ofstream out_file(out_path, std::ios::binary);

        encode(in_file, out_file, 1024 * 1024);

        in_file.close();
        out_file.close();

        return 0;
    }

};

#endif //MTF_SEQUENCECOMPRESSOR_H
