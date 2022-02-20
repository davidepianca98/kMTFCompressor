#ifndef MTF_MTFHASHTABLE_H
#define MTF_MTFHASHTABLE_H


#include <cstdint>
#include <iostream>
#include <vector>

class MTFHashTable {
private:
    // Hash table of MTF buffers
    std::vector<uint64_t> H;

    // Keep track of visited MTF buffers
    std::vector<bool> visited;

    // Length of k-mers
    int k;

    // Statistics
    double full_cells = 0;

    static void mtfShift(uint64_t& buf, uint8_t c, uint8_t i);

    static void mtfAppend(uint64_t& buf, uint8_t c);

    static uint8_t mtfExtract(uint64_t buf, uint8_t i);

    static uint8_t mtfEncode(uint64_t& buf, uint8_t c);

    static uint8_t mtfDecode(uint64_t& buf, uint8_t c);

    void keep_track(uint64_t hash);

public:
    explicit MTFHashTable(int k);

    void print_stats() const;

    void encode(const uint8_t *block, long size, uint8_t *out_block);

    void decode(const uint8_t *block, long size, uint8_t *out_block);
};


#endif //MTF_MTFHASHTABLE_H
