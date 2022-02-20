
#ifndef MTF_RABINFINGERPRINT_H
#define MTF_RABINFINGERPRINT_H


#include <cstdint>
#include <vector>
#include "Hash.h"

class RabinFingerprint : public Hash {
private:
    uint64_t x;
    uint64_t xk = 1;

    int k;

    // Rolling k-mer
    std::vector<uint8_t> kmer;

    uint64_t i = 0;

    //constexpr uint64_t M19 = (uint64_t(1)<<19)-1;
    //constexpr uint64_t M31 = (uint64_t(1)<<31)-1;
    // 1009, 1013, 1019, 10007, 10009, 10037, 100003, 100019, 100043, 1000003, 1000033, 1000037
    static constexpr uint64_t P10M = 10000019;
    static constexpr uint64_t P100M = 100000007;

public:
    static constexpr uint64_t q = 10007; // This gets around 50% hash table usage

    RabinFingerprint(int k, const std::vector<uint8_t> &start);

    void update(uint8_t c) override;
};


#endif //MTF_RABINFINGERPRINT_H
