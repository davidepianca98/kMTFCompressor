
#ifndef MTF_MTFHASH_H
#define MTF_MTFHASH_H


class MTFHash {

public:
    static int compress(const std::string& path, const std::string& out_path, int k);

    static int compress_stream(const std::string& path, const std::string& out_path, int k);

    static int decompress(const std::string& path, const std::string& out_path, int k);

    static int decompress_stream(const std::string& path, const std::string& out_path, int k);
};


#endif //MTF_MTFHASH_H
