
#ifndef MTF_CORE_H
#define MTF_CORE_H


#include <vector>
#include <cstdint>

class Core {

    bool valid = false;
    std::future<uint32_t> future;

public:
    std::vector<uint8_t> block;
    std::vector<uint8_t> out_block;

    Core(int in_block_size, int out_block_size);

    void start(uint32_t function(const uint8_t *block, long size, int k, uint8_t *final_block), long size, int k);

    uint32_t get();
};


#endif //MTF_CORE_H
