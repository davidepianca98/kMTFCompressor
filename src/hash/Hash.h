
#ifndef MTF_HASH_H
#define MTF_HASH_H


#include <cstdint>
#include <random>

class Hash {
protected:
    uint64_t hash = 0;

    int k;
    int i = 0;

    // Rolling k-mer
    std::vector<uint8_t> kmer;

    static constexpr uint64_t M61 = 2305843009213693951;

    std::mt19937_64 gen;
    std::uniform_int_distribution<uint64_t> dis;

    static inline uint64_t fast_modulo(uint64_t val) {
        uint64_t res = (val & M61) + (val >> 61);
        return (res >= M61) ? res - M61 : res;
    }

public:

    Hash(int k, uint64_t seed);

    virtual ~Hash() = default;

    virtual uint8_t update(uint8_t c);

    [[nodiscard]] inline uint64_t get_hash() const {
        return hash;
    }

    [[nodiscard]] inline uint64_t get_length() const {
        return k;
    }
};


#endif //MTF_HASH_H
