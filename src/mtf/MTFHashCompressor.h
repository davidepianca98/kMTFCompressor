
#ifndef MTF_MTFHASHCOMPRESSOR_H
#define MTF_MTFHASHCOMPRESSOR_H



class MTFHashCompressor {

    static uint64_t generate_seed();

public:
    static int get_cores();

    template <typename HASH, uint32_t SIZE>
    static int compress_block(const std::string& path, const std::string& out_path, int k, uint64_t max_memory_usage);

    template <typename HASH, uint32_t SIZE>
    static int decompress_block(const std::string& path, const std::string& out_path, int k, uint64_t max_memory_usage);

    template <typename HASH, uint32_t SIZE>
    static int compress_stream(const std::string& path, const std::string& out_path, int k, uint64_t max_memory_usage);

    template <typename HASH, uint32_t SIZE>
    static int decompress_stream(const std::string& path, const std::string& out_path, int k, uint64_t max_memory_usage);
};


#endif //MTF_MTFHASHCOMPRESSOR_H
