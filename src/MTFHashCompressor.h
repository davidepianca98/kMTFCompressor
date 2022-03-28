
#ifndef MTF_MTFHASHCOMPRESSOR_H
#define MTF_MTFHASHCOMPRESSOR_H


class MTFHashCompressor {

public:
    static int compress_block(const std::string& path, const std::string& out_path, int k);

    static int compress_stream(const std::string& path, const std::string& out_path, int k);

    static int decompress_block(const std::string& path, const std::string& out_path, int k);

    static int decompress_stream(const std::string& path, const std::string& out_path, int k);
};


#endif //MTF_MTFHASHCOMPRESSOR_H
