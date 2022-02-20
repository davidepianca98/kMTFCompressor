
#include <iostream>
#include <fastpfor/codecs.h>
#include <fastpfor/codecfactory.h>
#include "MTFHash.h"
#include "MTFHashTable.h"

std::string codec_name = "simdoptpfor"; // simdoptpfor, simdfastpfor256

void compress_final(const uint32_t *block, size_t size, uint32_t *out_block, size_t& compressed_size) {
    FastPForLib::IntegerCODEC &codec = *FastPForLib::CODECFactory::getFromName(codec_name);

    codec.encodeArray(block, size, out_block, compressed_size);
    compressed_size *= 4;
}

void enlarge(const uint8_t *block, long size, uint8_t *out_block) {
    for (int i = 0; i < size; i++) {
        out_block[i * 4] = block[i];
        out_block[(i * 4) + 1] = 0;
        out_block[(i * 4) + 2] = 0;
        out_block[(i * 4) + 3] = 0;
    }
}

void reduce(const uint8_t *block, long size, uint8_t *out_block) {
    for (int i = 0; i < size; i += 4) {
        out_block[i / 4] = block[i];
    }
}

uint32_t MTFHash::compressBlock(const uint8_t *block, long size, int k, uint8_t *final_block) {
    uint8_t out_block[size];
    auto *out_block2 = new uint8_t[size * 4]; // TODO handle differently, maybe use vector.data()

    MTFHashTable mtf(k);
    mtf.encode(block, size, out_block);

    //for (int i = 0; i < size; i++) {
    //    std::cout << uint64_t(out_block[i]) << " ";
    //}

    // TODO probably do this in mtf step
    enlarge(out_block, size, out_block2);

    size_t compressed_size = size * 4 + 1024; // TODO probably pass as parameter

    compress_final(reinterpret_cast<const uint32_t *>(out_block2), size, reinterpret_cast<uint32_t *>(final_block), compressed_size);
    delete[] out_block2;
    return (uint32_t) compressed_size;
}

int MTFHash::compress(const std::string& path, const std::string& out_path, int k) {
    std::ifstream in_file(path, std::ios::binary);
    if (in_file.fail()) {
        return 1;
    }
    std::ofstream out_file(out_path, std::ios::binary);

    int block_size = 1024 * 1024; // 1 MB block size
    while (in_file.good()) {
        uint8_t block[block_size];
        uint8_t out_block[block_size * 4 + 1024];

        in_file.read(reinterpret_cast<char *>(block), block_size);
        long read_bytes = in_file.gcount();
        if (read_bytes == 0) {
            break;
        }

        // TODO spawn threads that compress, maybe spawn with different k, and save in block header the best k and the compressed data
        uint32_t compressed_block_size = compressBlock(block, read_bytes, k, out_block);

        out_file.write(reinterpret_cast<const char *>(&compressed_block_size), 4);
        out_file.write(reinterpret_cast<const char *>(&k), 4);
        out_file.write(reinterpret_cast<const char *>(out_block), compressed_block_size);
    }

    in_file.close();
    out_file.close();

    return 0;
}

void decompress_final(const uint32_t *data, size_t size, uint32_t *out_block, size_t& decompressed_size) {
    FastPForLib::IntegerCODEC &codec = *FastPForLib::CODECFactory::getFromName(codec_name);

    codec.decodeArray(data, size, out_block, decompressed_size);
}

uint32_t MTFHash::decompressBlock(const uint8_t *block, long size, int k, uint8_t *final_block) {
    size_t decompressed_size = ((uint32_t *) block)[0] * 4 * 32 + 1024; // TODO probably move in final
    uint8_t out_block[decompressed_size * 4];
    decompress_final(reinterpret_cast<const uint32_t *>(block), size / 4, reinterpret_cast<uint32_t *>(out_block), decompressed_size);

    // TODO probably do this in mtf step
    auto *out_block2 = new uint8_t[decompressed_size]; // TODO find other way, on stack gives seg fault, maybe use vector.data()
    reduce(out_block, decompressed_size * 4, out_block2);

    MTFHashTable mtf(k);
    mtf.decode(out_block2, decompressed_size, final_block);
    delete[] out_block2;

    return (uint32_t) decompressed_size;
}

int MTFHash::decompress(const std::string &path, const std::string &out_path) {
    std::ifstream in_file(path, std::ios::binary);
    if (in_file.fail()) {
        return 1;
    }
    std::ofstream out_file(out_path, std::ios::binary);

    while (in_file.good()) {
        uint32_t block_size;
        in_file.read(reinterpret_cast<char *>(&block_size), 4);

        if (in_file.eof()) {
            break;
        }

        int k;
        in_file.read(reinterpret_cast<char *>(&k), 4);

        uint8_t block[block_size];
        uint8_t out_block[block_size * 4 + 1024];

        in_file.read(reinterpret_cast<char *>(block), block_size);
        long read_bytes = in_file.gcount(); // TODO check read_byte is equal to block_size

        // TODO spawn threads that decompress
        uint32_t compressed_block_size = decompressBlock(block, read_bytes, k, out_block);

        out_file.write(reinterpret_cast<const char *>(out_block), compressed_block_size);
    }

    in_file.close();
    out_file.close();

    return 0;
}
