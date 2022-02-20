
#include "MTFHashTable.h"
#include "hash/Hash.h"
#include "hash/RabinFingerprint.h"
#include "hash/DumbHash.h"
#include "hash/MinimiserHash.h"
#include "VectorHash.h"


void MTFHashTable::mtfShift(uint64_t& buf, uint8_t c, uint8_t i) {
    // If the position is zero, no need to change the buffer
    if (i != 0) {
        uint64_t left = (buf >> ((i + 1) * 8)) << ((i + 1) * 8); // Extract the part to be preserved
        buf = (buf << (8 - i) * 8) >> ((8 - i - 1) * 8); // Make space for the character in the first position and clean the leftmost bytes
        buf |= left | uint64_t(c); // Put character in the first position
    }
}

void MTFHashTable::mtfAppend(uint64_t& buf, uint8_t c) {
    buf = (buf << 8) | uint64_t(c);
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
uint8_t MTFHashTable::mtfEncode(uint64_t& buf, uint8_t c) {
    uint8_t i;

    if (c < 8) {
        //std::cout << "mah "; // TODO remove
    }

    for (i = 0; i < 8; i++) {
        if (mtfExtract(buf, i) == c) { // Check if the character in the i-th position from the right is equal to c
            mtfShift(buf, c, i);
            return i;
        }
    }

    // Not found so shift left and put character in first position
    mtfAppend(buf, c);

    return c;
}

uint8_t MTFHashTable::mtfDecode(uint64_t& buf, uint8_t i) {
    if (i >= 8) {
        mtfAppend(buf, i);
        return i;
    } else {
        uint8_t ca = mtfExtract(buf, i);
        mtfShift(buf, ca, i);

        return ca;
    }
}

void MTFHashTable::keep_track(uint64_t hash) {
    if (!visited.at(hash)) {
        full_cells += 1;
        visited.at(hash) = true;
    }
}

MTFHashTable::MTFHashTable(int k): H(RabinFingerprint::q, 0), visited(RabinFingerprint::q, false), k(k) {}

void MTFHashTable::print_stats() const {
    std::cout << "Used hash cells = " << full_cells << std::endl;
    std::cout << "Hash load = " << full_cells / double(RabinFingerprint::q) << std::endl;
}

void MTFHashTable::encode(const uint8_t *block, long size, uint8_t *out_block) {
    uint8_t c;

    std::vector<uint8_t> start(k);
    int i;
    for (i = 0; i < k; i++) {
        c = block[i];
        start[i] = c;
        out_block[i] = c;
    }

    // Rolling hash to access the table
    RabinFingerprint hash(k, start);

    while (i < size) {
        c = block[i];
        keep_track(hash.get_hash());

        uint8_t pos = mtfEncode(H[hash.get_hash()], c);
        out_block[i] = pos;
        hash.update(c); // TODO hashando sopra si ottengono run più lunghi però poi non so come decode
        i++;
    }
}

void MTFHashTable::decode(const uint8_t *block, long size, uint8_t *out_block) {
    uint8_t c;

    std::vector<uint8_t> start(k);
    int i;
    for (i = 0; i < k; i++) {
        c = block[i];
        start[i] = c;
        out_block[i] = c;
    }

    // Rolling hash to access the table
    RabinFingerprint hash(k, start);

    while (i < size) {
        c = block[i];
        uint8_t ca = mtfDecode(H[hash.get_hash()], c);
        out_block[i] = ca;

        hash.update(ca);
        i++;
    }
}
