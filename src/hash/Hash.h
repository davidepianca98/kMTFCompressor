
#ifndef MTF_HASH_H
#define MTF_HASH_H


#include <cstdint>

class Hash {
protected:
    uint64_t hash = 0;
    uint64_t size = 0;
    uint64_t window_size = 0;

    uint64_t k;

    // Rolling k-mer
    std::vector<uint8_t> kmer;
public:

    explicit Hash(uint64_t k, uint64_t window_size, uint64_t size) : k(k), size(size), window_size(window_size), kmer(k) {}

    Hash(const Hash& hash) : k(hash.k), size(hash.size), window_size(hash.window_size), kmer(hash.kmer) {}

    virtual ~Hash() = default;

    virtual void init(const std::vector<uint8_t> &start) {};

    virtual void update(uint8_t c) {};

    virtual void resize(uint64_t new_size) {};

    [[nodiscard]] virtual uint64_t get_hash() const {
        return hash;
    };

    [[nodiscard]] virtual uint64_t get_hash_full() const {
        return hash;
    };

    [[nodiscard]] uint64_t get_size() const {
        return size;
    }

    [[nodiscard]] uint64_t get_window_size() const {
        return window_size;
    }
};


#endif //MTF_HASH_H
