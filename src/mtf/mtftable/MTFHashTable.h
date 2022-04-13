#ifndef MTF_MTFHASHTABLE_H
#define MTF_MTFHASHTABLE_H


#include <cstdint>
#include <iostream>
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
    MTFRankBuffer<SIZE> *hash_table;
#else
    MTFBuffer<SIZE> *hash_table;
#endif
    uint32_t hash_table_size;
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

    MTFBuffer<SIZE>& linear_probe(MTFRankBuffer<SIZE> *table, uint32_t table_size, uint64_t key) {
        uint64_t hash = key & modulo_val;
        uint32_t i = 0;
        do {
            MTFBuffer<SIZE>& buf = table[(hash + i) & modulo_val];
            if (buf.get_key() == key || !buf.visited()) {
                return buf;
            }
            i++;
        } while (i < table_size);
    }

    uint32_t mtf_encode(uint8_t c) {
#ifdef MTF_STATS
        count_symbol_in(c);
#endif
        uint32_t out;

        if (kmer_chars < hash_function.get_length()) {
            kmer_chars++;
            out = (uint32_t) c + SIZE;
        } else {
            uint64_t key = hash_function.get_hash();
            MTFBuffer<SIZE>& buf = linear_probe(hash_table, hash_table_size, key);
            bool visited = buf.visited();
            out = buf.encode(c);
            keep_track(buf, key, visited);

#ifdef MTF_STATS
            distinct_kmers.insert(counter_hash.get_hash());
#endif
        }

        hash_function.update(c);

#ifdef MTF_STATS
        counter_hash.update(c);
        count_symbol_out(out);
#endif

        return out;
    }

    uint8_t mtf_decode(uint32_t i) {
        uint8_t c;
        if (kmer_chars < hash_function.get_length()) {
            kmer_chars++;
            c = (uint8_t) (i - SIZE);
        } else {
            uint64_t key = hash_function.get_hash();
            MTFBuffer<SIZE>& buf = linear_probe(hash_table, hash_table_size, key);
            bool visited = buf.visited();
            c = buf.decode(i);
            keep_track(buf, key, visited);
        }
        hash_function.update(c);

        return c;
    }

    inline void keep_track(MTFBuffer<SIZE>& buf, uint64_t key, bool visited) {
        if (!visited) {
            used_cells++;
            buf.set_visited(key);
            double_table();
        }
    }

    void count_symbol_in(uint8_t c) {
#ifdef MTF_STATS
        symbols_in[c]++;
        stream_length++;
#endif
    }

    void count_symbol_out(uint32_t i) {
#ifdef MTF_STATS
        symbols_out[i]++;
        if (i != last_symbol_out) {
            runs++;
            symbols_out_run[i]++;
        }
        last_symbol_out = i;
        if (i == 0) {
            zeros++;
        } else if (i == 1) {
            ones++;
        } else if (i == 2) {
            twos++;
        }
#endif
    }

    void double_table() {
        uint32_t new_size = hash_table_size * 2;
        if (doubling && used_cells * 1.25 > hash_table_size && new_size < max_table_size) {
            // Allocate table double the size of the older one
#ifdef MTF_RANK
            auto *hash_table_new = new MTFRankBuffer<SIZE>[new_size];
#else
            auto *hash_table_new = new MTFBuffer<SIZE>[new_size];
#endif
            // Calculate the new modulo based on the size
            modulo_val = UINT64_MAX >> (64 - (int) log2(new_size));

            // Rehashing approximation using the saved hash, it doesn't take into account the hash collisions that happened
            // earlier, but the only other way is to restart parsing the whole stream which is not feasible
            used_cells = 0;
            for (uint32_t i = 0; i < hash_table_size; i++) {
                MTFBuffer<SIZE>& buf = hash_table[i];
                if (buf.visited()) {
                    // The full hash is used to calculate the hash in the new bigger table with the new modulo
                    MTFBuffer<SIZE>& new_buf = linear_probe(hash_table_new, new_size, buf.get_key());
                    new_buf = buf;

                    used_cells++;
                }
            }
            delete[] hash_table;
            hash_table = hash_table_new;
            hash_table_size = new_size;
        }
    }

    double calculate_entropy(const uint64_t symbols[], int symbols_amount, int length) {
        double entropy = 0.0;
        for (int i = 0; i < symbols_amount; i++) {
            double p = (double) symbols[i] / length;
            if (p > 0) {
                entropy -= p * log2(p);
            }
        }
        return entropy;
    }

public:
    MTFHashTable(int block_size, uint64_t max_memory_usage, int k, uint64_t seed) : hash_function(k, seed), block_size(block_size), counter_hash(k, seed) {
        max_table_size = max_memory_usage / sizeof(MTFRankBuffer<SIZE>);
        if (doubling) { // TODO set as parameter
            hash_table_size = 256;
        } else {
            //hash_table_size = max_table_size;
            hash_table_size = 524288 * 16;
            //hash_table_size = 256;
        }
#ifdef MTF_RANK
        hash_table = new MTFRankBuffer<SIZE>[hash_table_size];
#else
        hash_table = new MTFBuffer<SIZE>[hash_table_size];
#endif

        modulo_val = UINT64_MAX >> (64 - (int) log2(hash_table_size));
    }

    ~MTFHashTable() {
        delete[] hash_table;
    }

    void print_stats() {
        std::cout << "Used hash cells = " << used_cells << "/" << hash_table_size << std::endl;
        std::cout << "Hash table load = " << used_cells / double(hash_table_size) << std::endl;
#ifdef MTF_STATS
        std::cout << "Number of distinct kmers = " << distinct_kmers.size() << ", Number of colliding kmers: " << distinct_kmers.size() - used_cells << std::endl;

        std::cout << "Number of runs = " << runs << std::endl;
        std::cout << "Number of zeros = " << zeros << ", Percentage of zeros = " << double(zeros) / double(stream_length) << std::endl;
        std::cout << "Number of ones = " << ones << ", Number of twos = " << twos << ", Percentage of zeros, ones, twos = " << double(zeros + ones + twos) / double(stream_length) << std::endl;

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

        std::cout << "Max compression size Entropy Coding = " << (uint64_t) (stream_length * entropy_out) / 8 << " bytes" << std::endl;
        std::cout << "Average run length = " << double(stream_length) / double(runs) << std::endl;
#endif
    }

};


#endif //MTF_MTFHASHTABLE_H
