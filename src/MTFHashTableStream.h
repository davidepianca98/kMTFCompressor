
#ifndef MTF_MTFHASHTABLESTREAM_H
#define MTF_MTFHASHTABLESTREAM_H


#include "MTFHashTable.h"
#include "stream/obitstream.h"
#include "stream/ibitstream.h"

template <typename T>
class MTFHashTableStream : public MTFHashTable<T> {

    std::vector<std::thread> threads;
    std::thread writer;

    std::vector<uint8_t> in_data;
    std::vector<uint32_t> mtf_out_data;

public:
    MTFHashTableStream(int k, int blockSize, Hash& hash);

    void encode(std::istream& in, obitstream& out);

    void decode(ibitstream& in, std::ostream& out);
};


#endif //MTF_MTFHASHTABLESTREAM_H
