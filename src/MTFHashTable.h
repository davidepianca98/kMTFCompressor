#ifndef MTF_MTFHASHTABLE_H
#define MTF_MTFHASHTABLE_H


#include <cstdint>
#include <iostream>
#include <vector>
#include <queue>
#include "SPSCQueue.h"

class MTFHashTable {
private:
    // Hash table of MTF buffers
    std::vector<uint64_t> hash_table;

    // Keep track of visited MTF buffers
    std::vector<bool> visited;

    // Length of k-mers
    int k;

    // Statistics
    double used_cells = 0;

    static void mtfShift(uint64_t& buf, uint8_t c, uint8_t i);

    static void mtfAppend(uint64_t& buf, uint8_t c);

    static uint8_t mtfExtract(uint64_t buf, uint8_t i);

    static uint32_t mtfEncode(uint64_t& buf, uint8_t c);

    static uint8_t mtfDecode(uint64_t& buf, uint32_t c);

    void keep_track(uint64_t hash);

    void thread_f(rigtorp::SPSCQueue<std::tuple<uint8_t, uint64_t, uint32_t *>> *q);

public:
    explicit MTFHashTable(int k);

    void print_stats() const;

    void encode(const uint8_t *block, long size, uint32_t *out_block);

    void encode2(std::istream& in, std::ostream& out);

    void encode3(std::istream& in, std::ostream& out);

    void decode(const uint32_t *block, long size, uint8_t *out_block);
};


#endif //MTF_MTFHASHTABLE_H
