#ifndef MTF_MTFHASHTABLE_H
#define MTF_MTFHASHTABLE_H


#include <cstdint>
#include <iostream>
#include <vector>
#include <unordered_set>
#include <cstring>
#include <cassert>
#include <immintrin.h>
#include "Hash.h"
#include "mtf/buffer/MTFBuffer.h"
#include "mtf/buffer/CountBuffer.h"
#include "Identity.h"
#include "randomized/TabulationHash.h"

#define MTF_STATS

template <typename BUFFER, uint32_t SIZE>
class MTFHashTable {
protected:
    static constexpr float MAX_LOAD_FACTOR = 0.9;

    // Hash table of MTF buffers
    BUFFER *hash_table;
    typedef uint32_t hash_type;

    uint32_t hash_table_size;
    TabulationHash hash_function;

    uint64_t max_table_size;

    uint64_t modulo_val;
    uint32_t kmer_chars = 0;

    // Statistics
    double used_cells = 0;

    uint64_t last_increment_size;
    uint64_t stream_length = 0;

#ifdef MTF_STATS
    // Keep track of number of symbols
    uint64_t symbols_in[256] = { 0 };
    uint64_t symbols_out[256 + SIZE] = { 0 };
    // Runs
    uint8_t last_symbol_out = 0;
    uint64_t runs = 1;
    uint64_t zero_runs = 0;
    uint64_t probes = 0;
    uint64_t probe_count = 0;
#endif

    uint32_t linear_probe(BUFFER *table, uint32_t table_size, hash_type key_hash, uint64_t key, bool count=false) {
#ifdef MTF_STATS
        if (count)
            probe_count++;
#endif
        uint64_t hash = key_hash & modulo_val;
        uint32_t i = 0;
        uint32_t index;
        do {
#ifdef MTF_STATS
            if (count)
                probes++;
#endif

            index = (hash + i) & modulo_val;
            if (!table[index].is_visited() || table[index].get_key() == key_hash) {
                return index;
            }
            i++;
        } while (i < table_size && (i < 8 || used_cells < hash_table_size * MAX_LOAD_FACTOR));

        table[index].set_visited(key_hash);

        return index;
    }

    inline void keep_track(uint32_t index, hash_type key_hash) {
        if (!hash_table[index].is_visited()) {
            used_cells++;
            hash_table[index].set_visited(key_hash);
            double_table();
        }

        /*if (kmer_chars < 6 && stream_length > 8 * last_increment_size) {
            std::cout << "Incrementing k to " << kmer_chars + 1 << " at stream length " << stream_length << std::endl;
            increment_k();
            last_increment_size *= 8;
        }*/
    }

    // Needs the whole key in the buffer, not just the hash
    void increment_k() {
        uint64_t old_modulo_val = modulo_val;
        uint64_t new_modulo_val = modulo_val;

        uint32_t new_size = hash_table_size;
        while (new_size < used_cells * SIZE) {
            new_size *= 2;
            new_modulo_val = (new_modulo_val << 1) | 1;
        }
#ifdef MTF_RANK
        auto *hash_table_new = new MTFRankBuffer<SIZE>[new_size];
#else
        auto *hash_table_new = new MTFBuffer<SIZE>[new_size];
#endif

        hash_function.increment_k();

        used_cells = 0;
        for (uint32_t i = 0; i < hash_table_size; i++) {
            if (hash_table[i].is_visited()) {
                for (int j = 0; j < hash_table[i].get_size(); j++) {
                    uint64_t new_key = hash_table[i].get_key() << 8 | hash_table[i].extract(j);
                    hash_type new_hash = hash_function.compute(new_key);

                    modulo_val = new_modulo_val;
                    uint32_t index = linear_probe(hash_table_new, new_size, new_hash, new_key);

                    modulo_val = old_modulo_val;
                    uint64_t suffix_key = new_key & ((1 << (kmer_chars * 8)) - 1);
                    uint32_t index_suffix = linear_probe(hash_table, hash_table_size, hash_function.compute(suffix_key), suffix_key);

                    hash_table_new[index].set_visited(new_key);
                    used_cells++;

                    for (int k = 0; k < hash_table[index_suffix].get_size(); k++) {
                        hash_table_new[index].append(hash_table[index_suffix].extract(k));
                    }
                }
            }
        }
        delete[] hash_table;
        hash_table = hash_table_new;
        hash_table_size = new_size;
        modulo_val = new_modulo_val;

        kmer_chars++;
    }

    void count_symbol_in(uint8_t c) {
#ifdef MTF_STATS
        symbols_in[c]++;
#endif
    }

    void count_symbol_out(uint32_t i) {
#ifdef MTF_STATS
        symbols_out[i]++;
        if (i != last_symbol_out) {
            runs++;
            if (i == 0) {
                zero_runs++;
            }
        }
        last_symbol_out = i;
#endif
    }

    void double_table() {
        uint32_t new_size = hash_table_size * 2;
        if (used_cells > hash_table_size * MAX_LOAD_FACTOR && new_size <= max_table_size) {
            // Allocate table double the size of the older one
            auto *hash_table_new = new BUFFER[new_size];

            // Calculate the new modulo based on the size
            modulo_val = (modulo_val << 1) | 1;

            // Rehashing using the saved key
            used_cells = 0;
            for (uint32_t i = 0; i < hash_table_size; i++) {
                if (hash_table[i].is_visited()) {
                    // Compute the new hash and find the new position in the table
                    hash_type hash = hash_table[i].get_key();
                    uint32_t index = linear_probe(hash_table_new, new_size, hash, hash_table[i].get_key());
                    hash_table_new[index] = hash_table[i];

                    used_cells++;
                }
            }
            delete[] hash_table;
            hash_table = hash_table_new;
            hash_table_size = new_size;
        }
    }

    double calculate_entropy(const uint64_t symbols[], int symbols_amount, uint64_t length) {
        double entropy = 0.0;
        for (int i = 0; i < symbols_amount; i++) {
            double p = (double) symbols[i] / double(length);
            if (p > 0) {
                entropy -= p * log2(p);
            }
        }
        return entropy;
    }

    uint32_t buffer_encode(BUFFER& buffer, uint8_t c) {
        for (uint16_t i = 0; i < buffer.get_size(); i++) {
            if (buffer.extract(i) == c) { // Check if the character in the i-th position from the right is equal to c
                buffer.shift(i);
                return i;
            }
        }

        // Not found so add the symbol to the buffer
        buffer.append(c);

        // Sum size to differentiate between indexes on the MTF buffer and characters
        return c + SIZE;
    }

    uint8_t buffer_decode(BUFFER& buffer, uint32_t symbol) {
        uint8_t c;
        if (symbol >= SIZE) {
            c = symbol - SIZE;
            buffer.append(c);
        } else {
            c = buffer.extract(symbol);
            buffer.shift(symbol);
        }
        return c;
    }

public:
    MTFHashTable(uint64_t max_memory_usage, int k, uint64_t seed) : hash_function(k, seed) {
        max_table_size = max_memory_usage / sizeof(BUFFER);
        hash_table_size = 256;
        hash_table = new BUFFER[hash_table_size];

        modulo_val = UINT64_MAX >> (64 - (int) log2(hash_table_size));

        last_increment_size = 256 * 8 * 8 * 8;
    }

    ~MTFHashTable() {
        delete[] hash_table;
    }

    uint32_t mtf_encode(uint8_t c) {
#ifdef MTF_STATS
        count_symbol_in(c);
#endif
        stream_length++;
        uint32_t out;

        if (kmer_chars < hash_function.get_length()) {
            kmer_chars++;
            out = (uint32_t) c + SIZE;
        } else {
            uint64_t key = hash_function.get_key();
            uint32_t index = linear_probe(hash_table, hash_table_size, hash_function.get_hash(), key, true);
            out = buffer_encode(hash_table[index], c);
            keep_track(index, hash_function.get_hash());
        }

        hash_function.update(c);

#ifdef MTF_STATS
        count_symbol_out(out);
#endif

        return out;
    }

    uint8_t mtf_decode(uint32_t i) {
#ifdef MTF_STATS
        count_symbol_in(i);
#endif
        stream_length++;
        uint8_t c;
        if (kmer_chars < hash_function.get_length()) {
            kmer_chars++;
            c = (uint8_t) (i - SIZE);
        } else {
            uint64_t key = hash_function.get_key();
            uint32_t index = linear_probe(hash_table, hash_table_size, hash_function.get_hash(), key, true);
            c = buffer_decode(hash_table[index], i);
            keep_track(index, hash_function.get_hash());
        }
        hash_function.update(c);

        return c;
    }

    void encode(const uint8_t *block, long size, uint32_t *out_block) {
        for (int i = 0; i < size; i++) {
            out_block[i] = mtf_encode(block[i]);
        }
    }

    void decode(const uint32_t *block, long size, uint8_t *out_block) {
        for (int i = 0; i < size; i++) {
            out_block[i] = mtf_decode(block[i]);
        }
    }

    void print_stats() {

        static uint64_t total = 0;
        static uint64_t total_overhead = 0;

        std::cout << "Used hash cells = " << used_cells << "/" << hash_table_size << std::endl;
        std::cout << "Hash table load = " << used_cells / double(hash_table_size) << std::endl;
#ifdef MTF_STATS
        std::cout << "Average probes = " << probes / double(probe_count) << std::endl;

        std::cout << "Number of zeros = " << symbols_out[0] << ", Percentage of zeros = " << double(symbols_out[0]) / double(stream_length) << std::endl;
        double entropy_in = calculate_entropy(symbols_in, 256, stream_length);
        double entropy_out = calculate_entropy(symbols_out, 256 + SIZE, stream_length);

        for (uint32_t i : symbols_in) {
            std::cout << i << ",";
        }
        std::cout << std::endl;
        for (uint32_t i : symbols_out) {
            std::cout << i << ",";
        }
        std::cout << std::endl;

        std::cout << "Entropy original = " << entropy_in << std::endl;
        std::cout << "Entropy MTF = " << entropy_out << std::endl;

        total += entropy_out * stream_length;
        std::cout << "Entropy sum MTF * |s| = " << total << std::endl;

        std::cout << "Max compression size Entropy Coding = " << (uint64_t) (stream_length * entropy_out) / 8 << " bytes" << std::endl;
        std::cout << "Number of runs = " << runs << ", Average run length = " << double(stream_length) / double(runs) << std::endl;
        std::cout << "Number of 0 runs = " << zero_runs << ", Average 0 run length = " << double(symbols_out[0]) / double(zero_runs) << std::endl;

        uint64_t sum_of_symbols = 0;
        for (uint32_t i = 0; i < hash_table_size; i++) {
            sum_of_symbols += hash_table[i].get_size();

            for (int j = 0; j < hash_table[i].get_size(); j++) {
                uint16_t sym = hash_table[i].extract(j) + SIZE;
                total_overhead += (uint64_t) floor(log2((double) sym)) + 2 * floor(log2(log2(sym) + 1)) + 1;
            }
        }
        std::cout << "Sum of |w_s|/|s| = " << double(sum_of_symbols) / double(stream_length) << std::endl;
        std::cout << "Total overhead in bits with Elias Delta = " << total_overhead << std::endl;
#endif
    }

};


#endif //MTF_MTFHASHTABLE_H
