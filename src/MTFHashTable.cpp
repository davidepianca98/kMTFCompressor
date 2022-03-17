
#include <future>
#include <thread>
#include <boost/multiprecision/cpp_int.hpp>
#include "MTFHashTable.h"
#include "hash/Hash.h"
#include "RabinKarp.h"

template <typename T>
void MTFHashTable<T>::mtfShift(T& buf, uint8_t c, uint8_t i) {
    // If the position is zero, no need to change the buffer
    if (i != 0) {
        int bits = (i + 1) * 8;
        T left = (buf >> bits) << bits; // Extract the part to be preserved
        bits = (byte_size() - i) * 8;
        buf = (buf << bits) >> (bits - 8); // Make space for the character in the first position and clean the leftmost bytes
        buf |= left | c; // Put character in the first position
    }
}

template <typename T>
void MTFHashTable<T>::mtfAppend(T& buf, uint8_t c) {
    buf = (buf << 8) | c;
}

template <typename T>
uint8_t MTFHashTable<T>::mtfExtract(const T& buf, uint8_t i) {
    return static_cast<uint8_t>((buf >> (i * 8)) & 0xFF);
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
    keep_track(hash_function.get_hash());
    T& buf = hash_table[hash_function.get_hash()];

    hash_function.update(c);
    count_symbol_in(c);

    for (uint8_t i = 0; i < byte_size(); i++) {
        uint8_t extracted = mtfExtract(buf, i);
        if (extracted == c) { // Check if the character in the i-th position from the right is equal to c
            mtfShift(buf, c, i);

            count_symbol_out(i);

            return i;
        }
    }

    // Not found so shift left and put character in first position
    mtfAppend(buf, c);

    count_symbol_out(c + byte_size());

    // Sum 8 to differentiate between indexes on the MTF buffer and characters
    return c + byte_size();
}

template <typename T>
uint8_t MTFHashTable<T>::mtfDecode(uint32_t i) {
    keep_track(hash_function.get_hash());
    T& buf = hash_table[hash_function.get_hash()];

    if (i >= byte_size()) {
        mtfAppend(buf, i - byte_size());
        hash_function.update(i - byte_size());
        return i - byte_size();
    } else {
        uint8_t ca = mtfExtract(buf, i);
        mtfShift(buf, ca, i);
        hash_function.update(ca);
        return ca;
    }
}

template <typename T>
void MTFHashTable<T>::keep_track(uint64_t hash) {
    if (!visited[hash]) { // TODO visited should be kept in same structure as table so no double miss just to keep track
        used_cells++;
        visited[hash] = true;
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
    }
    last_symbol_out = i;
}

template <typename T>
void MTFHashTable<T>::double_table() {
    if (used_cells * 10 / table_size > 2 && table_size < 100000000) {
        used_cells = 0; // TODO this on makes it worse, probably because the table becomes bigger earlier so it has less collisions
        table_size *= 2;
        hash_table.resize(table_size);
        std::fill(hash_table.begin(), hash_table.end(), 0);
        visited.resize(table_size);
        std::fill(visited.begin(), visited.end(), false);
        hash_function.resize(table_size);
    }
}

template <typename T>
MTFHashTable<T>::MTFHashTable(int k, int block_size, Hash& hash): table_size(hash.get_size()), hash_table(hash.get_size()), visited(hash.get_size(), false), k(k), block_size(block_size),
                                                                  hash_function(hash) {}

template <typename T>
void MTFHashTable<T>::print_stats() {
    std::cout << "Used hash cells = " << used_cells << std::endl;
    std::cout << "Hash table load = " << used_cells / double(hash_function.get_size()) << std::endl;
    std::cout << "Number of runs = " << runs << std::endl;
    std::cout << "Max compression size = " << (uint64_t) (runs + (runs * log2(stream_length / runs) / 8)) << std::endl; // TODO runs iniziale in teoria dovrebbe essere la stringa con solo la prima lettera di ogni run, compressa H0

    calculate_entropy();
    std::cout << "Entropy original = " << entropy_in << std::endl;
    std::cout << "Entropy MTF = " << entropy_out << std::endl;
}

template<typename T>
void MTFHashTable<T>::calculate_entropy() {
    entropy_in = 0;
    entropy_out = 0;
    for (int i = 0; i < 256; i++) {
        double p1 = (double) symbols_in[i] / stream_length;
        if (p1 > 0) {
            entropy_in -= p1 * log2(p1);
        }
    }
    for (int i = 0; i < 256 + byte_size(); i++) {
        double p2 = (double) symbols_out[i] / stream_length;
        if (p2 > 0) {
            entropy_out -= p2 * log2(p2);
        }
    }
}

template class MTFHashTable<uint16_t>;
template class MTFHashTable<uint32_t>;
template class MTFHashTable<uint64_t>;
template class MTFHashTable<boost::multiprecision::uint128_t>;
template class MTFHashTable<boost::multiprecision::uint256_t>;
template class MTFHashTable<boost::multiprecision::uint512_t>;
template class MTFHashTable<boost::multiprecision::uint1024_t>;
