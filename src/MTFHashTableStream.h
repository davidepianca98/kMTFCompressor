
#ifndef MTF_MTFHASHTABLESTREAM_H
#define MTF_MTFHASHTABLESTREAM_H


#include "MTFHashTable.h"
#include "encoders/AdaptiveHuffman.h"
#include "stream/obitstream/obitstream.h"
#include "stream/ibitstream/ibitstream.h"

template <typename HASH, typename T>
class MTFHashTableStream : public MTFHashTable<HASH, T> {

    std::vector<uint8_t> byte_array;
    std::vector<uint32_t> int_array;

    void entropy_rle_encode_n(const uint32_t *data, int bytes, AdaptiveHuffman& ahrle, AdaptiveHuffman& ah, obitstream& out) {
        if (bytes > 0) {
            int n = 4;
            uint64_t counter = 1;
            uint32_t last = data[0];
            ah.encode(last, out);
            for (int i = 1; i < bytes; i++) {
                if (data[i] == last) {
                    counter++;
                    if (counter >= 255) {
                        ahrle.encode(counter - n, out);
                        counter = 0;
                    } else if (counter <= n) {
                        ah.encode(last, out);
                    }
                } else {
                    if (counter >= n) {
                        ahrle.encode(counter - n, out);
                    }
                    ah.encode(data[i], out);
                    counter = 1;
                }
                last = data[i];
            }
            if (counter >= n) {
                ahrle.encode(counter - n, out);
            }
        }
        //ah.normalizeWeights();
    }

    void entropy_encode(const uint32_t *data, int length, AdaptiveHuffman& ah, obitstream& out) {
        for (int i = 0; i < length; i++) {
            ah.encode(data[i], out);
        }
    }

    void reverse_mtf(const uint32_t *data, int length, std::ostream &out) {
        for (int i = 0; i < length; i++) {
            byte_array[i] = MTFHashTable<HASH, T>::mtfDecode(data[i]);
        }
        out.write(reinterpret_cast<const char *>(byte_array.data()), (long) length);
    }

public:
    MTFHashTableStream(int block_size, uint64_t max_memory_usage, int k, uint64_t seed) : MTFHashTable<HASH, T>(block_size, max_memory_usage, k, seed) {
        byte_array.resize(MTFHashTable<HASH, T>::block_size);
        int_array.resize(MTFHashTable<HASH, T>::block_size);
    }

    void encode(std::istream& in, obitstream& out) {
        std::future<void> future;
        auto *out_block1 = new uint32_t[MTFHashTable<HASH, T>::block_size];

        AdaptiveHuffman ahrle(UINT8_MAX + 1);
        AdaptiveHuffman ah(256 + MTFHashTable<HASH, T>::byte_size() + 1);
        long read_bytes;
        do {
            // Read block
            in.read(reinterpret_cast<char *>(byte_array.data()), MTFHashTable<HASH, T>::block_size);
            read_bytes = in.gcount();

            // Apply transformation
            for (int i = 0; i < read_bytes; i++) {
                int_array[i] = MTFHashTable<HASH, T>::mtfEncode(byte_array[i]);
            }

            if (future.valid()) {
                future.wait();
            }

            memcpy(out_block1, int_array.data(), read_bytes * 4); // TODO probably write to obufbitstream and do final encoding in another class
            future = std::async(std::launch::async, &MTFHashTableStream<HASH, T>::entropy_rle_encode_n, this, out_block1, read_bytes, std::ref(ahrle), std::ref(ah), std::ref(out));
            //future = std::async(std::launch::async, &MTFHashTableStream<HASH, T>::entropy_encode, this, out_block1, read_bytes, std::ref(ah), std::ref(out));

        } while (read_bytes > 0);
        if (future.valid()) {
            future.wait();
        }
        ah.encode(256 + MTFHashTable<HASH, T>::byte_size(), out);
        out.flush_remaining();

        MTFHashTable<HASH, T>::print_stats();
        delete[] out_block1;
    }

    void decode(ibitstream& in, std::ostream& out) {
        std::future<void> future;
        auto *out_block1 = new uint32_t[MTFHashTable<HASH, T>::block_size * 2];

        AdaptiveHuffman ahrle(UINT8_MAX + 1);
        AdaptiveHuffman ah(256 + MTFHashTable<HASH, T>::byte_size() + 1);
        int i = 0;
        bool stop = false;
        int counter = 1;
        int last = -1;

        while (!stop) {
            int num = ah.decode(in);
            // Check if error happened or EOF symbol reached
            if (num < 0 || num == 256 + MTFHashTable<HASH, T>::byte_size() || !in.remaining()) {
                stop = true;
            } else {
                out_block1[i++] = num;
                // RLE decode
                if (last == num) {
                    counter++;
                    if (counter >= 4) {
                        int length = ahrle.decode(in);
                        for (int j = 0; j < length; j++) {
                            out_block1[i++] = num;
                        }
                        counter = 0;
                    }
                } else {
                    counter = 1;
                }
                last = num;
            }

            if (i >= MTFHashTable<HASH, T>::block_size || stop) {
                if (future.valid()) {
                    future.wait();
                }
                memcpy(int_array.data(), out_block1, i * 4);

                future = std::async(std::launch::async, &MTFHashTableStream<HASH, T>::reverse_mtf, this, int_array.data(), i, std::ref(out));
                i = 0;
            }
        }
        if (future.valid()) {
            future.wait();
        }

        delete[] out_block1;
    }
};


#endif //MTF_MTFHASHTABLESTREAM_H
