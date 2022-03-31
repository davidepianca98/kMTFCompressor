#ifndef MTF_MTFHASHTABLE_H
#define MTF_MTFHASHTABLE_H


#include <cstdint>
#include <iostream>
#include <vector>
#include <queue>
#include <boost/multiprecision/cpp_int.hpp>
#include "Hash.h"

template <typename T>
class MTFHashTable {
protected:
    // Hash table of MTF buffers
    std::vector<T> hash_table;
    Hash& hash_function;

    // Size of the block
    int block_size;

    uint64_t table_size;
    uint64_t modulo_val;

    // Statistics
    double used_cells = 0;
    // Keep track of visited MTF buffers
    std::vector<bool> visited;
    // Keep track of number of symbols
    uint64_t symbols_in[256] = { 0 };
    uint64_t symbols_out[256 + 128] = { 0 }; // TODO should be + byte_size()
    uint64_t symbols_out_run[256 + 128] = { 0 }; // TODO should be + byte_size()
    uint64_t stream_length = 0;
    // Runs
    uint8_t last_symbol_out = 0;
    uint64_t runs = 1;
    uint64_t zeros = 0;
    uint64_t ones = 0;
    uint64_t twos = 0;


    std::vector<std::vector<uint64_t>> counter;


    static void mtfShiftFront(T& buf, uint8_t c, uint8_t i);

    static void mtfShiftRank(T& buf, std::vector<uint64_t>& count, uint8_t c, uint8_t i);

    static void mtfAppend(T& buf, uint8_t c);

    void mtfAppendRank(T& buf, std::vector<uint64_t>& count, uint8_t c);

    static uint8_t mtfExtract(const T& buf, uint8_t i);

    uint32_t mtfEncode(uint8_t c);

    uint8_t mtfDecode(uint32_t c);

    void keep_track(uint64_t hash);

    void count_symbol_in(uint8_t c);

    void count_symbol_out(uint32_t i);

    void double_table();

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

    double calculate_entropy(const uint64_t symbols[], int length);

public:
    explicit MTFHashTable(int block_size, Hash& hash);

    void print_stats();

};


#endif //MTF_MTFHASHTABLE_H
