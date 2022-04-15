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
    int64_t *hash_table_keys;
    uint32_t hash_table_size;
    HASH hash_function;

    // Size of the block
    int block_size;
    uint64_t max_table_size;
    static constexpr bool doubling = true;

    uint64_t modulo_val;
    uint32_t kmer_chars = 0;

    // Statistics
    double used_cells = 0;

#ifdef MTF_STATS
    // Keep track of number of symbols
    uint64_t symbols_in[256] = { 0 };
    uint64_t symbols_out[256 + SIZE] = { 0 };
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

    uint32_t linear_probe_simd(const int64_t *keys, uint32_t table_size, int64_t key) {
        uint64_t hash = key & modulo_val;
        uint64_t mod = (modulo_val >> 2) << 2;
        int off = (0b1111 >> (hash & 3)) << (hash & 3);

        uint32_t i = 0;
        // Load key 4 times in vector
        const __m256d key_vec = _mm256_set1_pd(*((double *) &key));
        do {
            uint32_t index = (hash + i) & mod;

            // Load array into vector
            __m256d vec = _mm256_load_pd((const double *) &keys[index]);

            // Compare key with vector of keys
            __m256d eq = _mm256_cmp_pd(vec, key_vec, _CMP_EQ_OQ);
            int mask = _mm256_movemask_pd(eq);
            if (mask != 0) {
                uint32_t offset = _tzcnt_u32(mask);
                return index + offset;
            }

            // Set corresponding bit in mask if most significant bit of 64 bit integer is set, which means a negative
            // number, where only -1 is valid, so not found
            mask = _mm256_movemask_pd(vec) & off;
            if (mask != 0) {
                uint32_t offset = _tzcnt_u32(mask);
                return index + offset;
            }

            i += 4;
            off = 0b1111;
        } while (i < table_size);
    }

    uint32_t linear_probe(const int64_t *keys, uint32_t table_size, int64_t key) {
        uint64_t hash = key & modulo_val;
        uint32_t i = 0;
        do {
            uint32_t index = (hash + i) & modulo_val;
            int64_t j = keys[index];
            if (j == -1 || j == key) {
                return index;
            }
            i++;
        } while (i < table_size);

        throw std::runtime_error("No more space in the Hash Table");
    }

    inline void keep_track(uint32_t index, int64_t key) {
        if (hash_table_keys[index] == -1) {
            used_cells++;
            hash_table_keys[index] = key;
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
            // Align at 32 byte for SIMD
            auto *hash_table_keys_new = new (std::align_val_t(32)) int64_t[new_size];
            memset(hash_table_keys_new, 0xFF, new_size * 8);

            // Calculate the new modulo based on the size
            modulo_val = UINT64_MAX >> (64 - (int) log2(new_size));

            // Rehashing approximation using the saved hash, it doesn't take into account the hash collisions that happened
            // earlier, but the only other way is to restart parsing the whole stream which is not feasible
            used_cells = 0;
            for (uint32_t i = 0; i < hash_table_size; i++) {
                int64_t key = hash_table_keys[i];
                if (key != -1) {
                    // The full hash is used to calculate the hash in the new bigger table with the new modulo
                    uint32_t index = linear_probe(hash_table_keys_new, new_size, key);
                    hash_table_new[index] = hash_table[i];
                    hash_table_keys_new[index] = key;

                    used_cells++;
                }
            }
            delete[] hash_table;
            delete[] hash_table_keys;
            hash_table = hash_table_new;
            hash_table_keys = hash_table_keys_new;
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
        hash_table_keys = new (std::align_val_t(32)) int64_t[hash_table_size];
        memset(hash_table_keys, 0xFF, hash_table_size * 8);

        modulo_val = UINT64_MAX >> (64 - (int) log2(hash_table_size));
    }

    ~MTFHashTable() {
        delete[] hash_table;
        delete[] hash_table_keys;
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
            uint32_t index = linear_probe(hash_table_keys, hash_table_size, key);
            out = hash_table[index].encode(c);
            keep_track(index, key);

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
            uint32_t index = linear_probe(hash_table_keys, hash_table_size, key);
            c = hash_table[index].decode(i);
            keep_track(index, key);
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
