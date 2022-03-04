#ifndef MTF_MTFHASHTABLE_H
#define MTF_MTFHASHTABLE_H


#include <cstdint>
#include <iostream>
#include <vector>
#include <queue>
#include "SPSCQueue.h"

class MTFHashTable {
protected:
    // Hash table of MTF buffers
    std::vector<uint64_t> hash_table;

    // Length of k-mers
    int k;
    // Size of the block for FastPFOR
    int block_size;

    // Statistics
    double used_cells = 0;
    // Keep track of visited MTF buffers
    std::vector<bool> visited;


    static void mtfShift(uint64_t& buf, uint8_t c, uint8_t i);

    static void mtfAppend(uint64_t& buf, uint8_t c);

    static uint8_t mtfExtract(uint64_t buf, uint8_t i);

    static uint32_t mtfEncode(uint64_t& buf, uint8_t c);

    static uint8_t mtfDecode(uint64_t& buf, uint32_t c);

    void keep_track(uint64_t hash);

public:
    explicit MTFHashTable(int k, int block_size);

    void print_stats() const;

};


#endif //MTF_MTFHASHTABLE_H
