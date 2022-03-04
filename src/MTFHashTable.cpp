
#include <future>
#include <thread>
#include "MTFHashTable.h"
#include "hash/Hash.h"


void MTFHashTable::mtfShift(uint64_t& buf, uint8_t c, uint8_t i) {
    // If the position is zero, no need to change the buffer
    if (i != 0) {
        uint64_t left = (buf >> ((i + 1) * 8)) << ((i + 1) * 8); // Extract the part to be preserved
        buf = (buf << (8 - i) * 8) >> ((8 - i - 1) * 8); // Make space for the character in the first position and clean the leftmost bytes
        buf |= left | c; // Put character in the first position
    }
}

void MTFHashTable::mtfAppend(uint64_t& buf, uint8_t c) {
    buf = (buf << 8) | c;
}

uint8_t MTFHashTable::mtfExtract(uint64_t buf, uint8_t i) {
    return (buf >> (i * 8)) & 0xFF;
}

/*
 * MTF buffer containing at most 8 chars
 *
 * When called on a 64-bits buffer buf (8 chars, top char in least significant position) and character c,
 * looks for the rightmost occurrence of c in buf, moving it to the front and returning its position in the buffer
 * before moving c. If c is not found in buf, then c is returned.
 */
uint32_t MTFHashTable::mtfEncode(uint64_t& buf, uint8_t c) {
    for (uint8_t i = 0; i < 8; i++) {
        if (mtfExtract(buf, i) == c) { // Check if the character in the i-th position from the right is equal to c
            mtfShift(buf, c, i);
            return i;
        }
    }

    // Not found so shift left and put character in first position
    mtfAppend(buf, c);

    // Sum 8 to differentiate between indexes on the MTF buffer and characters
    return c + 8;
}

uint8_t MTFHashTable::mtfDecode(uint64_t& buf, uint32_t i) {
    if (i >= 8) {
        mtfAppend(buf, i - 8);
        return i - 8;
    } else {
        uint8_t ca = mtfExtract(buf, i);
        mtfShift(buf, ca, i);
        return ca;
    }
}

void MTFHashTable::keep_track(uint64_t hash) {
    if (!visited[hash]) {
        used_cells += 1;
        visited[hash] = true;
    }
}

// TODO size chosen based on the hash
MTFHashTable::MTFHashTable(int k, int block_size): hash_table(10000019, 0), visited(10000019, false), k(k), block_size(block_size) {}

void MTFHashTable::print_stats() const {
    std::cout << "Used hash cells = " << used_cells << std::endl;
    std::cout << "Hash table load = " << used_cells / double(hash_table.size()) << std::endl;
}
