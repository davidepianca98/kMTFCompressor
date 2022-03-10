
#ifndef MTF_HASH_H
#define MTF_HASH_H


#include <cstdint>

class Hash {
protected:
    uint64_t hash = 0;
    uint64_t size = 0;

    int k;

    // Rolling k-mer
    std::vector<uint8_t> kmer;
public:

    explicit Hash(uint64_t size, int k) : size(size), k(k), kmer(k) {}

    virtual ~Hash() = default;

    virtual void init(const std::vector<uint8_t> &start) {};

    virtual void update(uint8_t c) {};

    virtual void resize(uint64_t size) {};

    uint64_t get_hash() const {
        return hash;
    };

    uint64_t get_size() const {
        return size;
    }
};


#endif //MTF_HASH_H
