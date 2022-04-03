
#include <future>
#include <thread>
#include <boost/multiprecision/cpp_int.hpp>
#include "MTFHashTable.h"
#include "hash/Hash.h"


template <typename T>
MTFHashTable<T>::MTFHashTable(int block_size, Hash& hash) : hash_table(hash.get_size()), counter_rle(hash.get_size(), 0), block_size(block_size), hash_function(hash) {
    table_size = hash.get_size();
    modulo_val = UINT64_MAX >> (64 - (int) log2(table_size));
}


/*
 * MTF buffer containing at most 8 chars
 *
 * When called on a 64-bits buffer buf (8 chars, top char in least significant position) and character c,
 * looks for the rightmost occurrence of c in buf, moving it to the front and returning its position in the buffer
 * before moving c. If c is not found in buf, then c is returned.
 */
template <typename T>
uint32_t MTFHashTable<T>::mtfEncode(uint8_t c) {
    uint64_t hash = hash_function.get_hash_full() & modulo_val;
    MTFBuffer<T>& buf = hash_table[hash];
    keep_track(buf);

    hash_function.update(c);
    count_symbol_in(c);

    bool zero = false;
    for (uint8_t i = 0; i < byte_size(); i++) {
        uint8_t extracted = buf.extract(i);
        if (extracted == c) { // Check if the character in the i-th position from the right is equal to c
            buf.shift(c, i);

            count_symbol_out(i);

            return i;
        }
        // If two consecutive zeros are found, it means the remaining part of the buffer is not initialized yet, so
        // no need to continue iterating
        if (extracted == 0) {
            if (!zero) {
                zero = true;
            } else {
                break;
            }
        } else {
            zero = false;
        }
    }

    // Not found so shift left and put character in first position
    buf.append(c);

    count_symbol_out(c + byte_size());

    // Sum 8 to differentiate between indexes on the MTF buffer and characters
    return c + byte_size();
}

template <typename T>
uint8_t MTFHashTable<T>::mtfDecode(uint32_t i) {
    uint64_t hash = hash_function.get_hash_full() & modulo_val;
    MTFBuffer<T>& buf = hash_table[hash];
    keep_track(buf);

    uint8_t c;
    if (i >= byte_size()) {
        c = i - byte_size();
        buf.append(c);
    } else {
        c = buf.extract(i);
        buf.shift(c, i);
    }
    hash_function.update(c);
    return c;
}

template <typename T>
void MTFHashTable<T>::keep_track(MTFBuffer<T>& buf) {
    if (!buf.visited()) {
        used_cells++;
        buf.set_visited();
    }
}

template <typename T>
void MTFHashTable<T>::count_symbol_in(uint8_t c) {
    symbols_in[c]++;
    stream_length++;
}

template <typename T>
void MTFHashTable<T>::count_symbol_out(uint32_t i) {
    symbols_out[i]++;
    if (i != last_symbol_out) {
        runs++;
        symbols_out_run[i]++;
    }
    last_symbol_out = i;
    if (i == 0) {
        zeros++;
    } else if (i == 1) {
        ones++;
    } else if (i == 2) {
        twos++;
    }
}

template <typename T>
void MTFHashTable<T>::double_table() {
    if (used_cells * 10 > table_size && table_size < 134217728) {
        int old_table_size = table_size;
        table_size *= 2;
        hash_table.resize(table_size);
        hash_function.resize(table_size);

        modulo_val = UINT64_MAX >> (64 - (int) log2(table_size));
    }
}

template <typename T>
void MTFHashTable<T>::print_stats() {
    std::cout << "Used hash cells = " << used_cells << "/" << table_size << std::endl;
    std::cout << "Hash table load = " << used_cells / double(hash_function.get_size()) << std::endl;
    // TODO count number of distinct kmers

    std::cout << "Number of runs = " << runs << std::endl;
    std::cout << "Number of zeros = " << zeros << ", Percentage of zeros = " << double(zeros) / double(stream_length) << std::endl;
    std::cout << "Number of ones = " << ones << ", Number of twos = " << twos << ", Percentage of zeros, ones, twos = " << double(zeros + ones + twos) / double(stream_length) << std::endl;

    double entropy_in = calculate_entropy(symbols_in, 256);
    double entropy_out = calculate_entropy(symbols_out, 256 + byte_size());
    double entropy_out_rle = calculate_entropy(symbols_out_run, 256 + byte_size());

    std::cout << "Entropy original = " << entropy_in << std::endl;
    std::cout << "Entropy MTF = " << entropy_out << std::endl;
    std::cout << "Entropy MTF RLE = " << entropy_out_rle << std::endl;

    std::cout << "Max compression size Entropy Coding = " << (uint64_t) (stream_length * entropy_out) / 8 << " bytes" << std::endl;
    std::cout << "Max compression size RLE = " << (uint64_t) ((runs * entropy_out_rle) + (runs * ceil(2 * floor(log2(ceil(stream_length / runs)) + 1)))) / 8 << " bytes" << std::endl;
    std::cout << "Average run length = " << double(stream_length) / double(runs) << std::endl;
}

template<typename T>
double MTFHashTable<T>::calculate_entropy(const uint64_t symbols[], int length) {
    double entropy = 0.0;
    for (int i = 0; i < length; i++) {
        double p = (double) symbols[i] / stream_length;
        if (p > 0) {
            entropy -= p * log2(p);
        }
    }
    return entropy;
}

template class MTFHashTable<uint16_t>;
template class MTFHashTable<uint32_t>;
template class MTFHashTable<uint64_t>;
template class MTFHashTable<boost::multiprecision::uint128_t>;
template class MTFHashTable<boost::multiprecision::uint256_t>;
template class MTFHashTable<boost::multiprecision::uint512_t>;
template class MTFHashTable<boost::multiprecision::uint1024_t>;
