
#ifndef MTF_MTFHASHTABLESTREAM_H
#define MTF_MTFHASHTABLESTREAM_H


#include "MTFHashTable.h"
#include "Monitor.h"

template <typename T>
class MTFHashTableStream : public MTFHashTable<T> {

    Monitor mon;
    std::atomic_bool stop_thread;
    int num_cores;
    std::vector<std::thread> threads;
    std::thread writer;
    std::vector<uint64_t> hashes;

    bool started;

    uint8_t *in_data;
    uint32_t *mtf_out_data;
    int start_hash = 0;
    long read_bytes;
    long bytes_to_write;

    void encoder_thread(int thread_number);

    void writer_thread(std::ostream& out);

    void stop();

    void decompress(int decompressed_size, std::ostream& out, const uint32_t *in);

public:
    MTFHashTableStream(int k, int blockSize, Hash& hash);

    ~MTFHashTableStream();

    void encode(std::istream& in, std::ostream& out);

    void encode_pipeline(std::istream& in, std::ostream& out);

    void decode(std::istream& in, std::ostream& out);
};


#endif //MTF_MTFHASHTABLESTREAM_H
