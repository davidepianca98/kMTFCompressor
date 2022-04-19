
#ifndef MTF_ADAPTIVEELIASGAMMA_H
#define MTF_ADAPTIVEELIASGAMMA_H

#include <cstdint>
#include <numeric>
#include <vector>
#include "stream/obitstream/obitstream.h"
#include "stream/ibitstream/ibitstream.h"

class AdaptiveEliasGamma {
private:
    std::vector<uint64_t> count; // Keeps track of the frequencies of the symbols, indexed by symbol
    std::vector<uint32_t> rank; // Keeps track of the rank of the symbol, indexed by symbol
    std::vector<uint32_t> map; // Keeps track of the symbol by rank, indexed by rank

    void update_frequencies(uint32_t symbol);

public:
    explicit AdaptiveEliasGamma(int symbols_amount);

    inline static uint32_t fast_log2(uint32_t x) {
        return 31 - __builtin_clz(x);
    }

    void encode(uint32_t symbol, obitstream& out);

    void encode_array(const uint32_t *data, int size, obitstream& out);

    int decode(ibitstream& in);

    int decode_array(ibitstream& in, uint32_t *data, int size);

    static void elias_gamma_encode(uint32_t symbol, obitstream& out);

    static int elias_gamma_decode(ibitstream& in);

    static void elias_delta_encode(uint32_t symbol, obitstream& out);

    static int elias_delta_decode(ibitstream& in);
};

#endif //MTF_ADAPTIVEELIASGAMMA_H
