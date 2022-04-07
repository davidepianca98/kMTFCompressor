
#ifndef MTF_MTFHASHTABLESTREAM_H
#define MTF_MTFHASHTABLESTREAM_H


#include "MTFHashTable.h"
#include "encoders/AdaptiveHuffman.h"
#include "stream/obitstream.h"
#include "stream/ibitstream.h"

template <typename HASH, typename T>
class MTFHashTableStream : public MTFHashTable<HASH, T> {

    bool started;

    std::vector<uint8_t> byte_array;
    std::vector<uint32_t> int_array;

    void entropy_rle_encode_zeros(const uint32_t *data, int bytes, AdaptiveHuffman& ahrle, AdaptiveHuffman& ah, obitstream& out) {
        if (bytes > 0) {
            uint64_t counter = 0;
            for (int i = 0; i < bytes; i++) {
                if (data[i] == 0) {
                    counter++;
                    if (counter >= 255) {
                        ah.encode(0, out);
                        ahrle.encode(counter, out);
                        counter = 0;
                    }
                } else {
                    if (counter > 0) {
                        ah.encode(0, out);
                        ahrle.encode(counter, out);
                        counter = 0;
                    }
                    ah.encode(data[i], out);
                }
            }
            if (counter > 0) {
                ah.encode(0, out);
                ahrle.encode(counter, out);
            }
        }
    }

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
            if (counter > n) {
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
        std::vector<uint8_t> start(MTFHashTable<HASH, T>::hash_function.get_length());

        for (int i = 0; i < length; i++) {
            if (!started && i < start.size()) {
                start[i] = (uint8_t) (data[i] - MTFHashTable<HASH, T>::byte_size());
                byte_array[i] = start[i];
                if (i == start.size() - 1) {
                    started = true;
                    MTFHashTable<HASH, T>::hash_function.init(start);
                }
            } else {
                byte_array[i] = MTFHashTable<HASH, T>::mtfDecode(data[i]);
            }
        }
        out.write(reinterpret_cast<const char *>(byte_array.data()), (long) length);
    }

public:
    MTFHashTableStream(int blockSize, int k, uint64_t seed) : MTFHashTable<HASH, T>(blockSize, k, seed), started(false) {
        byte_array.resize(MTFHashTable<HASH, T>::block_size);
        int_array.resize(MTFHashTable<HASH, T>::block_size);
    }

    void encode(std::istream& in, obitstream& out) {
        started = false;
        std::vector<uint8_t> start(MTFHashTable<HASH, T>::hash_function.get_length());
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
                uint32_t out_c;
                if (!started && i < start.size()) {
                    uint8_t c = byte_array[i];
                    start[i] = c;
                    MTFHashTable<HASH, T>::count_symbol_in(c);
                    out_c = (uint32_t) c + MTFHashTable<HASH, T>::byte_size();
                    MTFHashTable<HASH, T>::count_symbol_out(out_c);
                    if (i == start.size() - 1) {
                        started = true;
                        MTFHashTable<HASH, T>::hash_function.init(start);
                    }
                } else {
                    out_c = MTFHashTable<HASH, T>::mtfEncode(byte_array[i]);
                }
                int_array[i] = out_c;
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
        started = false;
        std::future<void> future;
        auto *out_block1 = new uint32_t[MTFHashTable<HASH, T>::block_size];

        AdaptiveHuffman ah(256 + MTFHashTable<HASH, T>::byte_size() + 1);
        int i = 0;
        bool stop = false;
        while (!stop) {
            int num = ah.decode(in);
            // Check if error happened or EOF symbol reached
            if (num < 0 || num == 256 + MTFHashTable<HASH, T>::byte_size() || !in.remaining()) {
                stop = true;
            } else {
                out_block1[i++] = num;
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
