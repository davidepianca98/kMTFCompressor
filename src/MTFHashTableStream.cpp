
#include <boost/multiprecision/cpp_int.hpp>
#include <future>
#include "MTFHashTableStream.h"
#include "encoders/AdaptiveEliasGamma.h"


template <typename T>
MTFHashTableStream<T>::MTFHashTableStream(int blockSize, Hash& hash) : MTFHashTable<T>(blockSize, hash), started(false) {
    byte_array.resize(MTFHashTable<T>::block_size);
    int_array.resize(MTFHashTable<T>::block_size);
}


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

template <typename T>
void MTFHashTableStream<T>::entropy_encode(const uint32_t *data, int length, AdaptiveHuffman& ah, obitstream& out) {
    for (int i = 0; i < length; i++) {
        ah.encode(data[i], out);
    }
}


template <typename T>
void MTFHashTableStream<T>::encode(std::istream& in, obitstream& out) {
    started = false;
    std::vector<uint8_t> start(MTFHashTable<T>::hash_function.get_length());
    std::future<void> future;
    auto *out_block1 = new uint32_t[MTFHashTable<T>::block_size];

    AdaptiveHuffman ahrle(UINT8_MAX + 1);
    AdaptiveHuffman ah(256 + MTFHashTable<T>::byte_size() + 1);
    long read_bytes;
    do {
        // Read block
        in.read(reinterpret_cast<char *>(byte_array.data()), MTFHashTable<T>::block_size);
        read_bytes = in.gcount();

        // Apply transformation
        for (int i = 0; i < read_bytes; i++) {
            uint32_t out_c;
            if (!started && i < start.size()) {
                uint8_t c = byte_array[i];
                start[i] = c;
                MTFHashTable<T>::count_symbol_in(c);
                out_c = (uint32_t) c + MTFHashTable<T>::byte_size();
                MTFHashTable<T>::count_symbol_out(out_c);
                if (i == start.size() - 1) {
                    started = true;
                    MTFHashTable<T>::hash_function.init(start);
                }
            } else {
                out_c = MTFHashTable<T>::mtfEncode(byte_array[i]);
            }
            int_array[i] = out_c;
        }

        if (future.valid()) {
            future.wait();
        }

        memcpy(out_block1, int_array.data(), read_bytes * 4); // TODO probably write to obufbitstream and do final encoding in another class
        future = std::async(std::launch::async, entropy_rle_encode_n, out_block1, read_bytes, std::ref(ahrle), std::ref(ah), std::ref(out));
        //future = std::async(std::launch::async, &MTFHashTableStream<T>::entropy_encode, this, out_block1, read_bytes, std::ref(ah), std::ref(out));

    } while (read_bytes > 0);
    if (future.valid()) {
        future.wait();
    }
    ah.encode(256 + MTFHashTable<T>::byte_size(), out);
    out.flush_remaining();

    MTFHashTable<T>::print_stats();
    delete[] out_block1;
}

template <typename T>
void MTFHashTableStream<T>::reverse_mtf(const uint32_t *data, int length, std::ostream &out) {
    std::vector<uint8_t> start(MTFHashTable<T>::hash_function.get_length());

    for (int i = 0; i < length; i++) {
        if (!started && i < start.size()) {
            start[i] = (uint8_t) (data[i] - MTFHashTable<T>::byte_size());
            byte_array[i] = start[i];
            if (i == start.size() - 1) {
                started = true;
                MTFHashTable<T>::hash_function.init(start);
            }
        } else {
            byte_array[i] = MTFHashTable<T>::mtfDecode(data[i]);
        }
    }
    out.write(reinterpret_cast<const char *>(byte_array.data()), (long) length);
}

template <typename T>
void MTFHashTableStream<T>::decode(ibitstream &in, std::ostream &out) {
    started = false;
    std::future<void> future;
    auto *out_block1 = new uint32_t[MTFHashTable<T>::block_size];

    AdaptiveHuffman ah(256 + MTFHashTable<T>::byte_size() + 1);
    int i = 0;
    bool stop = false;
    while (!stop) {
        int num = ah.decode(in);
        // Check if error happened or EOF symbol reached
        if (num < 0 || num == 256 + MTFHashTable<T>::byte_size() || !in.remaining()) {
            stop = true;
        } else {
            out_block1[i++] = num;
        }

        if (i >= MTFHashTable<T>::block_size || stop) {
            if (future.valid()) {
                future.wait();
            }
            memcpy(int_array.data(), out_block1, i * 4);

            future = std::async(std::launch::async, &MTFHashTableStream<T>::reverse_mtf, this, int_array.data(), i, std::ref(out));
            i = 0;
        }
    }
    if (future.valid()) {
        future.wait();
    }

    delete[] out_block1;
}

template class MTFHashTableStream<uint16_t>;
template class MTFHashTableStream<uint32_t>;
template class MTFHashTableStream<uint64_t>;
template class MTFHashTableStream<boost::multiprecision::uint128_t>;
template class MTFHashTableStream<boost::multiprecision::uint256_t>;
template class MTFHashTableStream<boost::multiprecision::uint512_t>;
template class MTFHashTableStream<boost::multiprecision::uint1024_t>;
