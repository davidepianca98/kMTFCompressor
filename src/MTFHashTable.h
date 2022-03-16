#ifndef MTF_MTFHASHTABLE_H
#define MTF_MTFHASHTABLE_H


#include <cstdint>
#include <iostream>
#include <vector>
#include <queue>
#include <boost/multiprecision/cpp_int.hpp>
#include "SPSCQueue.h"
#include "Hash.h"

template <typename T>
class MTFHashTable {
protected:
    // Hash table of MTF buffers
    std::vector<T> hash_table;
    Hash& hash_function;

    // Length of k-mers
    int k;
    // Size of the block for FastPFOR
    int block_size;

    uint64_t table_size = 128;

    // Statistics
    double used_cells = 0;
    // Keep track of visited MTF buffers
    std::vector<bool> visited;
    // Keep track of number of symbols
    uint64_t symbols_in[256] = { 0 };
    uint64_t symbols_out[256 + 128] = { 0 }; // TODO should be + byte_size()
    uint64_t stream_length = 0;
    double entropy_in = 0;
    double entropy_out = 0;
    // Runs
    uint8_t last_symbol_out = 0;
    uint64_t runs = 0;


    static void mtfShift(T& buf, uint8_t c, uint8_t i);

    static void mtfAppend(T& buf, uint8_t c);

    static uint8_t mtfExtract(const T& buf, uint8_t i);

    uint32_t mtfEncode(uint8_t c);

    uint8_t mtfDecode(uint32_t c);

    int keep_track(uint64_t hash);

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

    void calculate_entropy();

public:
    explicit MTFHashTable(int k, int block_size, Hash& hash);

    void print_stats();

};


#endif //MTF_MTFHASHTABLE_H
