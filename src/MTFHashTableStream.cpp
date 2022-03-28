
#include <boost/multiprecision/cpp_int.hpp>
#include <future>
#include "MTFHashTableStream.h"
#include "encoders/AdaptiveEliasGamma.h"
#include "encoders/AdaptiveHuffman.h"


template <typename T>
MTFHashTableStream<T>::MTFHashTableStream(int k, int blockSize, Hash& hash) : MTFHashTable<T>(k, blockSize, hash){
    in_data.resize(this->block_size);
    mtf_out_data.resize(this->block_size);
}


void entropy_rle_encode(const uint32_t *data, int bytes, AdaptiveEliasGamma& aeg, AdaptiveHuffman& ah, obitstream& out) {
    if (bytes > 0) {
        uint32_t last = data[0];
        uint64_t counter = 1;
        for (int i = 1; i < bytes; i++) {
            if (last == data[i]) {
                counter++;
            } else {
                aeg.encode(counter, out);
                ah.encode(last, out);
                last = data[i];
                counter = 1;
            }
        }
        aeg.encode(counter, out);
        ah.encode(last, out);
    }
}

void entropy_encode(const uint32_t *data, int bytes, AdaptiveHuffman& ah, obitstream& out) {
    for (int i = 0; i < bytes; i++) {
        ah.encode(data[i], out);
    }
}

template <typename T>
void MTFHashTableStream<T>::encode(std::istream& in, obitstream& out) {
    bool started = false;
    std::vector<uint8_t> start(this->hash_function.get_window_size());
    std::future<void> future;
    auto *out_block1 = new uint32_t[this->block_size];

    //AdaptiveEliasGamma aeg(UINT16_MAX); // 256 + this->byte_size() for normal
    AdaptiveHuffman ah(256 + this->byte_size() + 1);
    long read_bytes;
    do {
        // Read block
        in.read(reinterpret_cast<char *>(in_data.data()), this->block_size);
        read_bytes = in.gcount();

        // Apply transformation
        for (int i = 0; i < read_bytes; i++) {
            uint32_t out_c;
            if (!started && i < start.size()) {
                uint8_t c = in_data[i];
                start[i] = c;
                this->count_symbol_in(c);
                out_c = (uint32_t) c + this->byte_size();
                this->count_symbol_out(out_c);
                if (i == start.size() - 1) {
                    started = true;
                    this->hash_function.init(start);
                }
            } else {
                out_c = this->mtfEncode(in_data[i]);
            }
            mtf_out_data[i] = out_c;

            this->double_table();
        }

        if (future.valid()) {
            future.wait();
        }

        memcpy(out_block1, mtf_out_data.data(), read_bytes * 4); // TODO probably write to obufbitstream and do final encoding in another class
        //future = std::async(std::launch::async, entropy_rle_encode, out_block1, read_bytes, std::ref(aeg), std::ref(ah), std::ref(out));
        future = std::async(std::launch::async, entropy_encode, out_block1, read_bytes, std::ref(ah), std::ref(out));

    } while (read_bytes > 0);
    if (future.valid()) {
        future.wait();
    }
    ah.encode(256 + this->byte_size(), out);
    out.flush_remaining();

    this->print_stats();
    delete[] out_block1;
}

template <typename T>
void MTFHashTableStream<T>::decode(ibitstream &in, std::ostream &out) {
    bool started = false;

    std::vector<uint8_t> start(this->hash_function.get_window_size());

    AdaptiveHuffman ah(256 + this->byte_size() + 1);
    int i = 0;
    while (in.remaining()) {
        uint32_t num = ah.decode(in);
        // Check if error happened or EOF symbol reached
        if (num < 0 || num == 256 + this->byte_size()) {
            break;
        }

        if (!started && i < start.size()) {
            start[i] = (uint8_t) (num - this->byte_size());
            out.put((char) start[i]);
            if (i == start.size() - 1) {
                started = true;
                this->hash_function.init(start);
            }
        } else {
            out.put(this->mtfDecode(num));
            this->double_table();
        }
        i++;
    }
}

template class MTFHashTableStream<uint16_t>;
template class MTFHashTableStream<uint32_t>;
template class MTFHashTableStream<uint64_t>;
template class MTFHashTableStream<boost::multiprecision::uint128_t>;
template class MTFHashTableStream<boost::multiprecision::uint256_t>;
template class MTFHashTableStream<boost::multiprecision::uint512_t>;
template class MTFHashTableStream<boost::multiprecision::uint1024_t>;
