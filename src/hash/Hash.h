//
// Created by 39340 on 23/01/2022.
//

#ifndef MTF_HASH_H
#define MTF_HASH_H


#include <cstdint>

class Hash {
protected:
    uint64_t hash = 0;
    uint32_t size = 0;
public:

    explicit Hash(uint32_t size) : size(size) {

    }

    virtual ~Hash() = default;

    virtual void update(uint8_t c) {};

    virtual uint64_t get_hash() const {
        return hash;
    };
};


#endif //MTF_HASH_H
