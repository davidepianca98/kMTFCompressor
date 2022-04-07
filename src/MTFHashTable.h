#ifndef MTF_MTFHASHTABLE_H
#define MTF_MTFHASHTABLE_H


#include <cstdint>
#include <iostream>
#include <vector>
#include <queue>
#include <boost/multiprecision/cpp_int.hpp>
#include "Hash.h"
#include "MTFBuffer.h"
#include "MTFRankBuffer.h"
#include "Identity.h"

#define MTF_RANK

template <typename HASH, typename T>
class MTFHashTable {
protected:
    // Hash table of MTF buffers
#ifdef MTF_RANK
    std::vector<MTFRankBuffer<T>> hash_table;
#else
    std::vector<MTFBuffer<T>> hash_table;
#endif
    HASH hash_function;

    // Size of the block
    int block_size;

    uint64_t modulo_val;

    constexpr static uint8_t byte_size() noexcept {
        if (std::is_same<T, boost::multiprecision::uint128_t>::value) {
            return 16;
        } else if (std::is_same<T, boost::multiprecision::uint256_t>::value) {
            return 32;
        } else if (std::is_same<T, boost::multiprecision::uint512_t>::value) {
            return 64;
        } else if (std::is_same<T, boost::multiprecision::uint1024_t>::value) {
            return 128;
        } else {
            return sizeof(T);
        }
    }

    // Statistics
    double used_cells = 0;
    // Keep track of number of symbols
    uint64_t symbols_in[256] = { 0 };
    uint64_t symbols_out[256 + byte_size()] = { 0 };
    uint64_t symbols_out_run[256 + byte_size()] = { 0 };
    uint64_t stream_length = 0;
    // Runs
    uint8_t last_symbol_out = 0;
    uint64_t runs = 1;
    uint64_t zeros = 0;
    uint64_t ones = 0;
    uint64_t twos = 0;


    uint32_t mtfEncode(uint8_t c) {
        uint64_t hash = hash_function.get_hash() & modulo_val;
        MTFBuffer<T>& buf = hash_table[hash];
        keep_track(buf);

        hash_function.update(c);
        count_symbol_in(c);

        uint32_t out = buf.encode(c);
        count_symbol_out(out);

        double_table();
        return out;
    }

    uint8_t mtfDecode(uint32_t i) {
        uint64_t hash = hash_function.get_hash() & modulo_val;
        MTFBuffer<T>& buf = hash_table[hash];
        keep_track(buf);

        uint8_t c = buf.decode(i);
        hash_function.update(c);

        double_table();
        return c;
    }

    void keep_track(MTFBuffer<T>& buf) {
        if (!buf.visited()) {
            used_cells++;
            buf.set_visited();
        }
    }

    void count_symbol_in(uint8_t c) {
        symbols_in[c]++;
        stream_length++;
    }

    void count_symbol_out(uint32_t i) {
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

    void double_table() {
        if (used_cells * 10 > hash_table.size() && hash_table.size() < 134217728) {
            hash_table.resize(hash_table.size() * 2);

            modulo_val = UINT64_MAX >> (64 - (int) log2(hash_table.size()));
        }
    }

    double calculate_entropy(const uint64_t symbols[], int length) {
        double entropy = 0.0;
        for (int i = 0; i < length; i++) {
            double p = (double) symbols[i] / stream_length;
            if (p > 0) {
                entropy -= p * log2(p);
            }
        }
        return entropy;
    }

public:
    MTFHashTable(int block_size, int k, uint64_t seed) : hash_table(4096),
                                                         block_size(block_size), hash_function(k, seed) {
        modulo_val = UINT64_MAX >> (64 - (int) log2(hash_table.size()));
    }

    void print_stats() {
        std::cout << "Used hash cells = " << used_cells << "/" << hash_table.size() << std::endl;
        std::cout << "Hash table load = " << used_cells / double(hash_table.size()) << std::endl;
        // TODO count number of distinct kmers

        std::cout << "Number of runs = " << runs << std::endl;
        std::cout << "Number of zeros = " << zeros << ", Percentage of zeros = " << double(zeros) / double(stream_length) << std::endl;
        std::cout << "Number of ones = " << ones << ", Number of twos = " << twos << ", Percentage of zeros, ones, twos = " << double(zeros + ones + twos) / double(stream_length) << std::endl;

        double entropy_in = calculate_entropy(symbols_in, 256);
        double entropy_out = calculate_entropy(symbols_out, 256 + byte_size());
        double entropy_out_rle = calculate_entropy(symbols_out_run, 256 + byte_size());

        for (uint32_t i : symbols_in) {
            std::cout << i << ",";
        }
        std::cout << std::endl;
        for (uint32_t i : symbols_out) {
            std::cout << i << ",";
        }
        std::cout << std::endl;

        std::cout << "Entropy original = " << entropy_in << std::endl;
        std::cout << "Entropy MTF = " << entropy_out << std::endl;
        std::cout << "Entropy MTF RLE = " << entropy_out_rle << std::endl;

        std::cout << "Max compression size Entropy Coding = " << (uint64_t) (stream_length * entropy_out) / 8 << " bytes" << std::endl;
        std::cout << "Max compression size RLE = " << (uint64_t) ((runs * entropy_out_rle) + (runs * ceil(2 * floor(log2(ceil(stream_length / runs)) + 1)))) / 8 << " bytes" << std::endl;
        std::cout << "Average run length = " << double(stream_length) / double(runs) << std::endl;
    }

};


#endif //MTF_MTFHASHTABLE_H
