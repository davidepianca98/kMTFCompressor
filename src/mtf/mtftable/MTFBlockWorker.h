
#ifndef MTF_MTFBLOCKWORKER_H
#define MTF_MTFBLOCKWORKER_H


#include <cstdint>
#include <future>
#include <vector>
#include <boost/multiprecision/cpp_int.hpp>

template <typename HASH, typename T>
class MTFBlockWorker {

    int k;
    uint64_t seed;
    uint64_t max_memory_usage;
    bool valid = false;
    std::future<uint32_t> future;

    std::vector<uint8_t> in_block;
    std::vector<uint8_t> out_block;

    uint32_t compressBlock(const uint8_t *in, int size, uint8_t *final_block);

    uint32_t decompressBlock(uint8_t *in, int size, uint8_t *final_block);

    constexpr static uint8_t byte_size() noexcept {
        if (std::is_same<T, boost::multiprecision::uint128_t>::value) {
            return 16;
        } else if (std::is_same<T, boost::multiprecision::uint256_t>::value) {
            return 32;
        } else if (std::is_same<T, boost::multiprecision::uint512_t>::value) {
            return 64;
        } else if (std::is_same<T, boost::multiprecision::uint1024_t>::value) {
            return 128;
        } else {
            return sizeof(T);
        }
    }

public:

    MTFBlockWorker(int k, uint64_t seed, int in_block_size, int out_block_size, uint64_t max_memory_usage);

    void startCompression(long size);

    void startDecompression(long size);

    uint32_t get();

    uint8_t *get_in_block();

    int get_in_block_size();

    uint8_t const *get_out_block();
};


#endif //MTF_MTFBLOCKWORKER_H
