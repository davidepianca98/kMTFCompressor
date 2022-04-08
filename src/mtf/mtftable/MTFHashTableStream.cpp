
#include <future>
#include "MTFHashTableStream.h"
#include "randomized/RabinKarp.h"
#include "encoders/RunLength.h"

template <typename HASH, typename T>
MTFHashTableStream<HASH, T>::MTFHashTableStream(int block_size, uint64_t max_memory_usage, int k, uint64_t seed) : MTFHashTable<HASH, T>(block_size, max_memory_usage, k, seed) {
    byte_array.resize(MTFHashTable<HASH, T>::block_size);
    int_array.resize(MTFHashTable<HASH, T>::block_size);
}

template <typename HASH, typename T>
void MTFHashTableStream<HASH, T>::encode(std::istream& in, obitstream& out) {
    std::future<void> future;
    auto *out_block1 = new uint32_t[MTFHashTable<HASH, T>::block_size];

    RunLength rle(256 + MTFHashTable<HASH, T>::byte_size() + 1);
    long read_bytes;
    do {
        // Read block
        in.read(reinterpret_cast<char *>(byte_array.data()), MTFHashTable<HASH, T>::block_size);
        read_bytes = in.gcount();

        // Apply transformation
        for (int i = 0; i < read_bytes; i++) {
            int_array[i] = MTFHashTable<HASH, T>::mtf_encode(byte_array[i]);
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
    rle.encode(256 + MTFHashTable<HASH, T>::byte_size(), out);
    out.flush_remaining();

    MTFHashTable<HASH, T>::print_stats();
    delete[] out_block1;
}

template <typename HASH, typename T>
void MTFHashTableStream<HASH, T>::reverse_mtf(const uint32_t *data, int length, std::ostream &out) {
    for (int i = 0; i < length; i++) {
        byte_array[i] = MTFHashTable<HASH, T>::mtf_decode(data[i]);
    }
    out.write(reinterpret_cast<const char *>(byte_array.data()), (long) length);
}

template <typename HASH, typename T>
void MTFHashTableStream<HASH, T>::decode(ibitstream& in, std::ostream& out) {
    std::future<void> future;
    auto *out_block1 = new uint32_t[MTFHashTable<HASH, T>::block_size * 2];

    RunLength rle(256 + MTFHashTable<HASH, T>::byte_size() + 1);
    int read;

    do {
        read = rle.decode_array(in, out_block1, MTFHashTable<HASH, T>::block_size, 256 + MTFHashTable<HASH, T>::byte_size());

        if (future.valid()) {
            future.wait();
        }
        memcpy(int_array.data(), out_block1, read * 4);

        future = std::async(std::launch::async, &MTFHashTableStream<HASH, T>::reverse_mtf, this, int_array.data(), read, std::ref(out));
    } while (read > 0);
    if (future.valid()) {
        future.wait();
    }

    delete[] out_block1;
}

template class MTFHashTableStream<RabinKarp, uint16_t>;
template class MTFHashTableStream<RabinKarp, uint32_t>;
template class MTFHashTableStream<RabinKarp, uint64_t>;
template class MTFHashTableStream<RabinKarp, boost::multiprecision::uint128_t>;
template class MTFHashTableStream<RabinKarp, boost::multiprecision::uint256_t>;
template class MTFHashTableStream<RabinKarp, boost::multiprecision::uint512_t>;
template class MTFHashTableStream<RabinKarp, boost::multiprecision::uint1024_t>;
