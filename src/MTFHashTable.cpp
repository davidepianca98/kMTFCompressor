
#include <future>
#include <thread>
#include <boost/multiprecision/cpp_int.hpp>
#include "MTFHashTable.h"
#include "hash/Hash.h"

template <typename T>
void MTFHashTable<T>::mtfShift(T& buf, uint8_t c, uint8_t i) {
    // If the position is zero, no need to change the buffer
    if (i != 0) { // TODO optimize
        T left = (buf >> ((i + 1) * 8)) << ((i + 1) * 8); // Extract the part to be preserved
        buf = (buf << (byte_size() - i) * 8) >> ((byte_size() - i - 1) * 8); // Make space for the character in the first position and clean the leftmost bytes
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
    T& buf = hash_table[hash_function.get_hash()];
    keep_track(hash_function.get_hash());

    hash_function.update(c);
    symbols_in[c]++;
    stream_length++;

    for (uint8_t i = 0; i < byte_size(); i++) {
        uint8_t extracted = mtfExtract(buf, i);
        if (extracted == c) { // Check if the character in the i-th position from the right is equal to c
            mtfShift(buf, c, i);

            if (i != last_symbol_out) {
                runs++;
            }
            last_symbol_out = i;
            symbols_out[i]++;

            return i;
        }
    }

    // Not found so shift left and put character in first position
    mtfAppend(buf, c);

    if (c != last_symbol_out) {
        runs++;
    }
    last_symbol_out = c;
    symbols_out[c + 8]++;

    // Sum 8 to differentiate between indexes on the MTF buffer and characters
    return c + 8;
}

template <typename T>
uint8_t MTFHashTable<T>::mtfDecode(uint32_t i) {
    T& buf = hash_table[hash_function.get_hash()];
    keep_track(hash_function.get_hash());

    if (i >= 8) {
        mtfAppend(buf, i - 8);
        hash_function.update(i - 8);
        return i - 8;
    } else {
        uint8_t ca = mtfExtract(buf, i);
        mtfShift(buf, ca, i);
        hash_function.update(ca);
        return ca;
    }
}

template <typename T>
void MTFHashTable<T>::keep_track(uint64_t hash) {
    if (!visited[hash]) {
        used_cells++;
        visited[hash] = true;

        /*if (used_cells * 100 / table_size > 20 && table_size < 134217728) {
            table_size *= 2;
            std::cout << table_size << std::endl;
            hash_table.resize(table_size);
            visited.resize(table_size);
            hash_function.resize(table_size);
        }*/
    }
}

template <typename T>
MTFHashTable<T>::MTFHashTable(int k, int block_size, Hash& hash): table_size(128), hash_table(100000007), visited(100000007, false), k(k), block_size(block_size),
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
    for (int i = 0; i < 256 + 8; i++) {
        double p2 = (double) symbols_out[i] / stream_length;
        if (p2 > 0) {
            entropy_out -= p2 * log2(p2);
        }
    }
}

template class MTFHashTable<uint64_t>;
template class MTFHashTable<boost::multiprecision::uint1024_t>;
