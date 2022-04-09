
#include <future>
#include <cstring>
#include "MTFHashTableStream.h"
#include "randomized/RabinKarp.h"
#include "encoders/RunLength.h"
#include "randomized/LinearHash.h"
#include "randomized/MinimiserHash.h"

template <typename HASH, uint32_t SIZE>
MTFHashTableStream<HASH, SIZE>::MTFHashTableStream(int block_size, uint64_t max_memory_usage, int k, uint64_t seed) : MTFHashTable<HASH, SIZE>(block_size, max_memory_usage, k, seed) {
    byte_array.resize(MTFHashTable<HASH, SIZE>::block_size);
    int_array.resize(MTFHashTable<HASH, SIZE>::block_size);
}

template <typename HASH, uint32_t SIZE>
void MTFHashTableStream<HASH, SIZE>::encode(std::istream& in, obitstream& out) {
    std::future<void> future;
    auto *out_block1 = new uint32_t[MTFHashTable<HASH, SIZE>::block_size];

    RunLength rle(256 + SIZE + 1);
    long read_bytes;
    do {
        // Read block
        in.read(reinterpret_cast<char *>(byte_array.data()), MTFHashTable<HASH, SIZE>::block_size);
        read_bytes = in.gcount();

        // Apply transformation
        for (int i = 0; i < read_bytes; i++) {
            int_array[i] = MTFHashTable<HASH, SIZE>::mtf_encode(byte_array[i]);
        }

        if (future.valid()) {
            future.wait();
        }

        memcpy(out_block1, int_array.data(), read_bytes * 4);
        future = std::async(std::launch::async, &RunLength::encode_array, &rle, out_block1, read_bytes, std::ref(out));
        //future = std::async(std::launch::async, &AdaptiveHuffman::encode, &ah, out_block1, read_bytes, std::ref(out));
    } while (read_bytes > 0);
    if (future.valid()) {
        future.wait();
    }
    rle.encode_end(256 + SIZE, out);
    out.flush_remaining();

    MTFHashTable<HASH, SIZE>::print_stats();
    delete[] out_block1;
}

template <typename HASH, uint32_t SIZE>
void MTFHashTableStream<HASH, SIZE>::reverse_mtf(const uint32_t *data, int length, std::ostream &out) {
    for (int i = 0; i < length; i++) {
        byte_array[i] = MTFHashTable<HASH, SIZE>::mtf_decode(data[i]);
    }
    out.write(reinterpret_cast<const char *>(byte_array.data()), (long) length);
}

template <typename HASH, uint32_t SIZE>
void MTFHashTableStream<HASH, SIZE>::decode(ibitstream& in, std::ostream& out) {
    std::future<void> future;
    auto *out_block1 = new uint32_t[MTFHashTable<HASH, SIZE>::block_size];

    RunLength rle(256 + SIZE + 1);
    int read;

    do {
        read = rle.decode_array(in, out_block1, MTFHashTable<HASH, SIZE>::block_size, 256 + SIZE);
        if (future.valid()) {
            future.wait();
        }
        memcpy(int_array.data(), out_block1, read * 4);

        future = std::async(std::launch::async, &MTFHashTableStream<HASH, SIZE>::reverse_mtf, this, int_array.data(), read, std::ref(out));
    } while (read > 0);
    if (future.valid()) {
        future.wait();
    }

    delete[] out_block1;
}

template class MTFHashTableStream<RabinKarp, 2>;
template class MTFHashTableStream<RabinKarp, 4>;
template class MTFHashTableStream<RabinKarp, 6>;
template class MTFHashTableStream<RabinKarp, 8>;
template class MTFHashTableStream<RabinKarp, 16>;
template class MTFHashTableStream<RabinKarp, 32>;
template class MTFHashTableStream<RabinKarp, 64>;
template class MTFHashTableStream<RabinKarp, 128>;
template class MTFHashTableStream<RabinKarp, 256>;

template class MTFHashTableStream<LinearHash, 8>;
template class MTFHashTableStream<MinimiserHash<RabinKarp, LinearHash, RabinKarp>, 8>;
