
#ifndef MTF_DUMBHASH_H
#define MTF_DUMBHASH_H


#include <vector>
#include "Hash.h"

class DumbHash : public Hash {

    int i = 0;

public:
    explicit DumbHash(int k);

    void init(const std::vector<uint8_t> &start) override;

    void resize(uint64_t size) override;

    void update(uint8_t c) override;
};


#endif //MTF_DUMBHASH_H
