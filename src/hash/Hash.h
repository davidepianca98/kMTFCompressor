
#ifndef MTF_HASH_H
#define MTF_HASH_H


#include <cstdint>
#include <random>

class Hash {
protected:
    uint64_t hash = 0;

    uint64_t k;

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

    Hash(uint64_t k, uint64_t seed) : k(k), kmer(k, 0), gen(seed), dis(0, 100000000) {}

    virtual ~Hash() = default;

    virtual void init(const std::vector<uint8_t> &start) {};

    virtual void update(uint8_t c) {};

    [[nodiscard]] virtual uint64_t get_hash() const {
        return hash;
    };

    [[nodiscard]] uint64_t get_length() const {
        return k;
    }
};


#endif //MTF_HASH_H
