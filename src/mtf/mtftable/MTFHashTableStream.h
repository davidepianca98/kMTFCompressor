
#ifndef MTF_MTFHASHTABLESTREAM_H
#define MTF_MTFHASHTABLESTREAM_H


#include <cstring>
#include "MTFHashTable.h"
#include "encoders/AdaptiveHuffman.h"
#include "stream/obitstream/obitstream.h"
#include "stream/ibitstream/ibitstream.h"
#include "encoders/AdaptiveArithmetic.h"

template <typename BUFFER, uint32_t SIZE>
class MTFHashTableStream : public MTFHashTable<BUFFER, SIZE> {

    static constexpr int BLOCK_SIZE = 1024 * 1024;

    std::vector<uint8_t> byte_array;
    std::vector<uint32_t> int_array;


    void reverse_mtf(const uint32_t *data, int length, std::ostream& out) {
        for (int i = 0; i < length; i++) {
            byte_array[i] = MTFHashTable<BUFFER, SIZE>::mtf_decode(data[i]);
        }
        out.write(reinterpret_cast<const char *>(byte_array.data()), length);
    }

public:
    MTFHashTableStream(uint64_t max_memory_usage, int k, uint64_t seed) : MTFHashTable<BUFFER, SIZE>(max_memory_usage, k, seed) {
        byte_array.resize(BLOCK_SIZE);
        int_array.resize(BLOCK_SIZE);
    }

    void encode(std::istream& in, obitstream& out) {
        std::future<void> future;
        auto *out_block1 = new uint32_t[BLOCK_SIZE];

        RunLength rle(256 + SIZE + 1);
        //AdaptiveArithmetic aa(256 + SIZE + 1);
        long read_bytes;
        do {
            // Read block
            in.read(reinterpret_cast<char *>(byte_array.data()), BLOCK_SIZE);
            read_bytes = in.gcount();

            // Apply transformation
            for (int i = 0; i < read_bytes; i++) {
                int_array[i] = MTFHashTable<BUFFER, SIZE>::mtf_encode(byte_array[i]);
            }

            if (future.valid()) {
                future.wait();
            }

            memcpy(out_block1, int_array.data(), read_bytes * 4);
            future = std::async(std::launch::async, &RunLength::encode_array, &rle, out_block1, read_bytes, std::ref(out));
        } while (read_bytes > 0);
        if (future.valid()) {
            future.wait();
        }
        rle.encode_end(256 + SIZE, out);
        out.flush_remaining();

        MTFHashTable<BUFFER, SIZE>::print_stats();
        delete[] out_block1;
    }

    void decode(ibitstream& in, std::ostream& out) {
        std::future<void> future;
        auto *out_block1 = new uint32_t[BLOCK_SIZE];

        RunLength rle(256 + SIZE + 1);
        int read;

        do {
            read = rle.decode_array(in, out_block1, BLOCK_SIZE, 256 + SIZE);
            if (future.valid()) {
                future.wait();
            }
            memcpy(int_array.data(), out_block1, read * 4);

            future = std::async(std::launch::async, &MTFHashTableStream<BUFFER, SIZE>::reverse_mtf, this, int_array.data(), read, std::ref(out));
        } while (read > 0);
        if (future.valid()) {
            future.wait();
        }

        delete[] out_block1;
    }
};


#endif //MTF_MTFHASHTABLESTREAM_H
