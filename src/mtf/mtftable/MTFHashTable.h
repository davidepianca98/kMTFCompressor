#ifndef MTF_MTFHASHTABLE_H
#define MTF_MTFHASHTABLE_H


#include <cstdint>
#include <vector>
#include <unordered_set>
#include "Hash.h"
#include "mtf/buffer/MTFBuffer.h"
#include "mtf/buffer/MTFRankBuffer.h"
#include "Identity.h"

#define MTF_RANK
//#define MTF_STATS

template <typename HASH, uint32_t SIZE>
class MTFHashTable {
protected:
    // Hash table of MTF buffers
#ifdef MTF_RANK
    std::vector<MTFRankBuffer<SIZE>> hash_table;
#else
    std::vector<MTFBuffer<T>> hash_table;
#endif
    HASH hash_function;

    // Size of the block
    int block_size;
    uint64_t max_table_size;
    bool doubling = true;

    uint64_t modulo_val;
    uint32_t kmer_chars = 0;

    // Statistics
    double used_cells = 0;

#ifdef MTF_STATS
    // Keep track of number of symbols
    uint64_t symbols_in[256] = { 0 };
    uint64_t symbols_out[256 + SIZE] = { 0 };
    uint64_t symbols_out_run[256 + SIZE] = { 0 };
    uint64_t stream_length = 0;
    std::unordered_set<uint64_t> distinct_kmers;
    // Runs
    uint8_t last_symbol_out = 0;
    uint64_t runs = 1;
    uint64_t zeros = 0;
    uint64_t ones = 0;
    uint64_t twos = 0;
#endif
    Identity counter_hash;

    uint32_t mtf_encode(uint8_t c);

    uint8_t mtf_decode(uint32_t i);

    void keep_track(MTFBuffer<SIZE>& buf, uint64_t hash);

    void count_symbol_in(uint8_t c);

    void count_symbol_out(uint32_t i);

    void double_table();

    double calculate_entropy(const uint64_t symbols[], int length);

public:
    MTFHashTable(int block_size, uint64_t max_memory_usage, int k, uint64_t seed);

    void print_stats();

};


#endif //MTF_MTFHASHTABLE_H
