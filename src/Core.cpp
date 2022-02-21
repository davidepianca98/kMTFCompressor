
#include <future>
#include "Core.h"

Core::Core(int in_block_size, int out_block_size) : block(in_block_size), out_block(out_block_size) {}

void Core::start(uint32_t function(const uint8_t *block, long size, int k, uint8_t *final_block), long size, int k) {
    if (!valid && size > 0) {
        future = std::async(std::launch::async, function, block.data(), size, k, out_block.data());
        valid = true;
    }
}

uint32_t Core::get() {
    if (valid) {
        uint32_t res = future.get();
        valid = false;
        return res;
    }
    return 0;
}
