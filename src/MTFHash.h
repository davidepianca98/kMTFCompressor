
#ifndef MTF_MTFHASH_H
#define MTF_MTFHASH_H


class MTFHash {

    static uint32_t compressBlock(const uint8_t *block, long size, int k, uint8_t *final_block);

    static uint32_t decompressBlock(const uint8_t *block, long size, int k, uint8_t *final_block);

public:
    static int compress(const std::string& path, const std::string& out_path, int k);

    static int decompress(const std::string& path, const std::string& out_path);
};


#endif //MTF_MTFHASH_H
