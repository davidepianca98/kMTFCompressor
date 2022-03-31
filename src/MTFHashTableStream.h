
#ifndef MTF_MTFHASHTABLESTREAM_H
#define MTF_MTFHASHTABLESTREAM_H


#include "MTFHashTable.h"
#include "encoders/AdaptiveHuffman.h"
#include "stream/obitstream.h"
#include "stream/ibitstream.h"

template <typename T>
class MTFHashTableStream : public MTFHashTable<T> {

    bool started;

    std::vector<uint8_t> byte_array;
    std::vector<uint32_t> int_array;

    void entropy_encode(const uint32_t *data, int length, AdaptiveHuffman& ah, obitstream& out);

    void reverse_mtf(const uint32_t *data, int length, std::ostream &out);

public:
    MTFHashTableStream(int blockSize, Hash& hash);

    void encode(std::istream& in, obitstream& out);

    void decode(ibitstream& in, std::ostream& out);
};


#endif //MTF_MTFHASHTABLESTREAM_H
