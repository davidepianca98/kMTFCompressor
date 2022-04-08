
#ifndef MTF_MTFHASHTABLESTREAM_H
#define MTF_MTFHASHTABLESTREAM_H


#include "MTFHashTable.h"
#include "encoders/AdaptiveHuffman.h"
#include "stream/obitstream/obitstream.h"
#include "stream/ibitstream/ibitstream.h"

template <typename HASH, typename T>
class MTFHashTableStream : public MTFHashTable<HASH, T> {

    std::vector<uint8_t> byte_array;
    std::vector<uint32_t> int_array;


    void reverse_mtf(const uint32_t *data, int length, std::ostream &out);

public:
    MTFHashTableStream(int block_size, uint64_t max_memory_usage, int k, uint64_t seed);

    void encode(std::istream& in, obitstream& out);

    void decode(ibitstream& in, std::ostream& out);
};


#endif //MTF_MTFHASHTABLESTREAM_H
