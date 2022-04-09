
#include <iostream>
#include "MTFHashTable.h"
#include "randomized/RabinKarp.h"
#include "randomized/LinearHash.h"
#include "randomized/MinimiserHash.h"

template <typename HASH, uint32_t SIZE>
uint32_t MTFHashTable<HASH, SIZE>::mtf_encode(uint8_t c) {
#ifdef MTF_STATS
    count_symbol_in(c);
#endif
    uint32_t out;

    if (kmer_chars < hash_function.get_length()) {
        kmer_chars++;
        out = (uint32_t) c + SIZE;
    } else {
        uint64_t hash = hash_function.get_hash() & modulo_val;
        MTFBuffer<SIZE>& buf = hash_table[hash];
        keep_track(buf, hash_function.get_hash());
        out = buf.encode(c);

#ifdef MTF_STATS
        distinct_kmers.insert(counter_hash.get_hash());
#endif
    }

    hash_function.update(c);

#ifdef MTF_STATS
    counter_hash.update(c);
    count_symbol_out(out);
#endif

    double_table();
    return out;
}

template <typename HASH, uint32_t SIZE>
uint8_t MTFHashTable<HASH, SIZE>::mtf_decode(uint32_t i) {
    uint8_t c;
    if (kmer_chars < hash_function.get_length()) {
        kmer_chars++;
        c = (uint8_t) (i - SIZE);
    } else {
        uint64_t hash = hash_function.get_hash() & modulo_val;
        MTFBuffer<SIZE> &buf = hash_table[hash];
        keep_track(buf, hash_function.get_hash());

        c = buf.decode(i);
    }
    hash_function.update(c);

    double_table();
    return c;
}

template <typename HASH, uint32_t SIZE>
void MTFHashTable<HASH, SIZE>::keep_track(MTFBuffer<SIZE>& buf, uint64_t hash) {
    if (!buf.visited()) {
        used_cells++;
        buf.set_visited(hash);
    }
}

template <typename HASH, uint32_t SIZE>
void MTFHashTable<HASH, SIZE>::count_symbol_in(uint8_t c) {
#ifdef MTF_STATS
    symbols_in[c]++;
    stream_length++;
#endif
}

template <typename HASH, uint32_t SIZE>
void MTFHashTable<HASH, SIZE>::count_symbol_out(uint32_t i) {
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

template <typename HASH, uint32_t SIZE>
void MTFHashTable<HASH, SIZE>::double_table() {
    if (doubling && used_cells * 10 > hash_table.size() && hash_table.size() * 2 < max_table_size) {
        // Allocate table double the size of the older one
#ifdef MTF_RANK
        std::vector<MTFRankBuffer<SIZE>> hash_table_new(hash_table.size() * 2);
#else
        std::vector<MTFBuffer<SIZE>> hash_table_new(hash_table.size() * 2);
#endif
        // Calculate the new modulo based on the size
        modulo_val = UINT64_MAX >> (64 - (int) log2(hash_table_new.size()));

        // Rehashing approximation using the saved hash, it doesn't take into account the hash collisions that happened
        // earlier, but the only other way is to restart parsing the whole stream which is not feasible
        used_cells = 0;
        for (auto buf : hash_table) {
            if (buf.visited()) {
                // The full hash is used to calculate the hash in the new bigger table with the new modulo
                uint64_t hash = buf.get_key() & modulo_val;
                hash_table_new[hash] = buf;
                used_cells++;
            }
        }
        hash_table = hash_table_new;
    }
}

template <typename HASH, uint32_t SIZE>
double MTFHashTable<HASH, SIZE>::calculate_entropy(const uint64_t symbols[], int length) {
#ifdef MTF_STATS
    double entropy = 0.0;
    for (int i = 0; i < length; i++) {
        double p = (double) symbols[i] / stream_length;
        if (p > 0) {
            entropy -= p * log2(p);
        }
    }
    return entropy;
#endif
}

template <typename HASH, uint32_t SIZE>
MTFHashTable<HASH, SIZE>::MTFHashTable(int block_size, uint64_t max_memory_usage, int k, uint64_t seed) : hash_function(k, seed), block_size(block_size), counter_hash(k, seed) {
    max_table_size = max_memory_usage / sizeof(MTFRankBuffer<SIZE>);
    if (doubling) { // TODO set as parameter
        hash_table.resize(4096);
    } else {
        //hash_table.resize(max_table_size);
        hash_table.resize(524288 * 16);
        //hash_table.resize(4096);
    }

    modulo_val = UINT64_MAX >> (64 - (int) log2(hash_table.size()));
}

template <typename HASH, uint32_t SIZE>
void MTFHashTable<HASH, SIZE>::print_stats() {
    std::cout << "Used hash cells = " << used_cells << "/" << hash_table.size() << std::endl;
    std::cout << "Hash table load = " << used_cells / double(hash_table.size()) << std::endl;
#ifdef MTF_STATS
    std::cout << "Number of distinct kmers = " << distinct_kmers.size() << ", Number of colliding kmers: " << distinct_kmers.size() - used_cells << std::endl;

    std::cout << "Number of runs = " << runs << std::endl;
    std::cout << "Number of zeros = " << zeros << ", Percentage of zeros = " << double(zeros) / double(stream_length) << std::endl;
    std::cout << "Number of ones = " << ones << ", Number of twos = " << twos << ", Percentage of zeros, ones, twos = " << double(zeros + ones + twos) / double(stream_length) << std::endl;

    double entropy_in = calculate_entropy(symbols_in, 256);
    double entropy_out = calculate_entropy(symbols_out, 256 + SIZE);

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

template class MTFHashTable<RabinKarp, 2>;
template class MTFHashTable<RabinKarp, 4>;
template class MTFHashTable<RabinKarp, 6>;
template class MTFHashTable<RabinKarp, 8>;
template class MTFHashTable<RabinKarp, 16>;
template class MTFHashTable<RabinKarp, 32>;
template class MTFHashTable<RabinKarp, 64>;
template class MTFHashTable<RabinKarp, 128>;
template class MTFHashTable<RabinKarp, 256>;

template class MTFHashTable<LinearHash, 8>;
template class MTFHashTable<MinimiserHash<RabinKarp, LinearHash, RabinKarp>, 8>;
