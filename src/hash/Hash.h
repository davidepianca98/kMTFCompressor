
#ifndef MTF_HASH_H
#define MTF_HASH_H


#include <cstdint>
#include <random>

class Hash {
protected:
    uint64_t hash = 0;

    int k;

    uint64_t kmer_hash = 0;
    uint8_t *kmer_hash_p = reinterpret_cast<uint8_t *>(&kmer_hash);
    int last_index;

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

    [[nodiscard]] inline uint64_t get_key() const {
        return kmer_hash;
    }

    [[nodiscard]] inline uint64_t get_length() const {
        return k;
    }

    void increment_k();

    virtual uint64_t compute(uint64_t key);
};


#endif //MTF_HASH_H
