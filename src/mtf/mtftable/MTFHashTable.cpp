
#include "MTFHashTable.h"
#include "randomized/RabinKarp.h"

template <typename HASH, typename T>
uint32_t MTFHashTable<HASH, T>::mtf_encode(uint8_t c) {
#ifdef MTF_STATS
    count_symbol_in(c);
#endif
    uint32_t out;

    if (kmer_chars < hash_function.get_length()) {
        kmer_chars++;
        out = (uint32_t) c + byte_size();
    } else {
        uint64_t hash = hash_function.get_hash() & modulo_val;
        MTFBuffer<T>& buf = hash_table[hash];
        keep_track(buf);
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

template <typename HASH, typename T>
uint8_t MTFHashTable<HASH, T>::mtf_decode(uint32_t i) {
    uint8_t c;
    if (kmer_chars < hash_function.get_length()) {
        kmer_chars++;
        c = (uint8_t) (i - byte_size());
    } else {
        uint64_t hash = hash_function.get_hash() & modulo_val;
        MTFBuffer<T> &buf = hash_table[hash];
        keep_track(buf);

        c = buf.decode(i);
    }
    hash_function.update(c);

    double_table();
    return c;
}

template <typename HASH, typename T>
void MTFHashTable<HASH, T>::keep_track(MTFBuffer<T>& buf) {
    if (!buf.visited()) {
        used_cells++;
        buf.set_visited();
    }
}

template <typename HASH, typename T>
void MTFHashTable<HASH, T>::count_symbol_in(uint8_t c) {
#ifdef MTF_STATS
    symbols_in[c]++;
    stream_length++;
#endif
}

template <typename HASH, typename T>
void MTFHashTable<HASH, T>::count_symbol_out(uint32_t i) {
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

template <typename HASH, typename T>
void MTFHashTable<HASH, T>::double_table() {
    if (doubling && used_cells * 10 > hash_table.size() && hash_table.size() * 2 < max_table_size) {
        hash_table.resize(hash_table.size() * 2);

        modulo_val = UINT64_MAX >> (64 - (int) log2(hash_table.size()));
    }
}

template <typename HASH, typename T>
double MTFHashTable<HASH, T>::calculate_entropy(const uint64_t symbols[], int length) {
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

template <typename HASH, typename T>
MTFHashTable<HASH, T>::MTFHashTable(int block_size, uint64_t max_memory_usage, int k, uint64_t seed) : hash_function(k, seed), block_size(block_size), counter_hash(k, seed) {
    max_table_size = max_memory_usage / sizeof(MTFRankBuffer<T>);
    if (doubling) { // TODO set as parameter
        hash_table.resize(4096);
    } else {
        hash_table.resize(max_table_size);
        //hash_table.resize(524288);
        //hash_table.resize(4096);
    }

    modulo_val = UINT64_MAX >> (64 - (int) log2(hash_table.size()));
}

template <typename HASH, typename T>
void MTFHashTable<HASH, T>::print_stats() {
    std::cout << "Used hash cells = " << used_cells << "/" << hash_table.size() << std::endl;
    std::cout << "Hash table load = " << used_cells / double(hash_table.size()) << std::endl;
#ifdef MTF_STATS
    std::cout << "Number of distinct kmers = " << distinct_kmers.size() << ", Number of colliding kmers: " << distinct_kmers.size() - used_cells << std::endl;

    std::cout << "Number of runs = " << runs << std::endl;
    std::cout << "Number of zeros = " << zeros << ", Percentage of zeros = " << double(zeros) / double(stream_length) << std::endl;
    std::cout << "Number of ones = " << ones << ", Number of twos = " << twos << ", Percentage of zeros, ones, twos = " << double(zeros + ones + twos) / double(stream_length) << std::endl;

    double entropy_in = calculate_entropy(symbols_in, 256);
    double entropy_out = calculate_entropy(symbols_out, 256 + byte_size());
    double entropy_out_rle = calculate_entropy(symbols_out_run, 256 + byte_size());

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
    std::cout << "Entropy MTF RLE = " << entropy_out_rle << std::endl;

    std::cout << "Max compression size Entropy Coding = " << (uint64_t) (stream_length * entropy_out) / 8 << " bytes" << std::endl;
    std::cout << "Max compression size RLE = " << (uint64_t) ((runs * entropy_out_rle) + (runs * ceil(2 * floor(log2(ceil(stream_length / runs)) + 1)))) / 8 << " bytes" << std::endl;
    std::cout << "Average run length = " << double(stream_length) / double(runs) << std::endl;
#endif
}

template class MTFHashTable<RabinKarp, uint16_t>;
template class MTFHashTable<RabinKarp, uint32_t>;
template class MTFHashTable<RabinKarp, uint64_t>;
template class MTFHashTable<RabinKarp, boost::multiprecision::uint128_t>;
template class MTFHashTable<RabinKarp, boost::multiprecision::uint256_t>;
template class MTFHashTable<RabinKarp, boost::multiprecision::uint512_t>;
template class MTFHashTable<RabinKarp, boost::multiprecision::uint1024_t>;
