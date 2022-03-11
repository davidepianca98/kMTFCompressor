
#ifndef MTF_RABINFINGERPRINT_H
#define MTF_RABINFINGERPRINT_H


#include <cstdint>
#include <vector>
#include "Hash.h"

class RabinFingerprint : public Hash {
private:
    // Random number
    uint64_t x = 0;
    // Multiplier to shift left
    uint64_t xk = 1;

    uint64_t i = 0;



    //constexpr uint64_t M19 = (uint64_t(1)<<19)-1;
    //constexpr uint64_t M31 = (uint64_t(1)<<31)-1;
    // 1009, 1013, 1019, 10007, 10009, 10037, 100003, 100019, 100043, 1000003, 1000033, 1000037
    static constexpr uint64_t P10M = 10000019;
    static constexpr uint64_t P100M = 100000007;

    static constexpr uint64_t q = P100M;//2147483647; // 2^31 - 1

public:
    explicit RabinFingerprint(int k);

    void init(const std::vector<uint8_t> &start) override;

    void resize(uint64_t size) override;

    void update(uint8_t c) override;

    void increment_k(uint8_t c);

    void decrement_k();
};


#endif //MTF_RABINFINGERPRINT_H
