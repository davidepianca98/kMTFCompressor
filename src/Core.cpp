
#include <future>
#include <fastpfor/codecs.h>
#include <fastpfor/codecfactory.h>
#include <thread>
#include "Core.h"
#include "MTFHashTableBlock.h"
#include "RabinKarp.h"
#include "DumbHash.h"

Core::Core(int k, int in_block_size, int out_block_size) : k(k), block(in_block_size), out_block(out_block_size) {}


void Core::compress_final(const uint32_t *block, size_t size, uint32_t *out_block, size_t& compressed_size) {
    using namespace FastPForLib;
    // simdoptpfor, simdfastpfor256 are the best
    IntegerCODEC *codec = new CompositeCodec<SIMDOPTPFor<4, Simple16<false>>, VariableByte>();
    //IntegerCODEC *codec = new CompositeCodec<SIMDFastPFor<8>, VariableByte>();

    codec->encodeArray(block, size, out_block, compressed_size);
    compressed_size *= 4;
    delete codec;
}

uint32_t Core::compressBlock(const uint8_t *in_block, int size, uint8_t *final_block) const {
    auto *out_block1 = new uint32_t[size];

    //RabinKarp hash(k, 10000007);
    RabinKarp hash(k, 4096);
    MTFHashTableBlock<uint64_t> mtf(k, size, hash);
    mtf.encode(in_block, size, out_block1);

    //mtf.print_stats();

    //for (int i = 0; i < size; i++) {
        //std::cout << out_block[i] << " ";
    //}

    size_t compressed_size = size * 4 + 1024;
    compress_final(out_block1, size, reinterpret_cast<uint32_t *>(final_block), compressed_size);
    delete[] out_block1;
    return (uint32_t) compressed_size;
}

void Core::decompress_final(const uint32_t *data, size_t size, uint32_t *out_block, size_t& decompressed_size) {
    using namespace FastPForLib;
    IntegerCODEC *codec = new CompositeCodec<SIMDOPTPFor<4, Simple16<false>>, VariableByte>(); // TODO con true salva da solo la dimensione dell'input

    codec->decodeArray(data, size, out_block, decompressed_size);
    delete codec;
}

uint32_t Core::decompressBlock(const uint8_t *in_block, int size, uint8_t *final_block) const {
    size_t decompressed_size = ((uint32_t *) in_block)[0] * 4 * 32 + 1024;
    auto *out_block1 = new uint32_t[decompressed_size];
    decompress_final(reinterpret_cast<const uint32_t *>(in_block), size / 4, out_block1, decompressed_size);

    RabinKarp hash(k, 10000007);
    //DumbHash hash(k, 4095);
    MTFHashTableBlock<uint64_t> mtf(k, size, hash);
    mtf.decode(out_block1, (long) decompressed_size, final_block);

    delete[] out_block1;
    return (uint32_t) decompressed_size;
}


void Core::startCompression(long size) {
    if (!valid && size > 0) {
        future = std::async(std::launch::async, &Core::compressBlock, this, block.data(), size, out_block.data());
        valid = true;
    }
}

void Core::startDecompression(long size) {
    if (!valid && size > 0) {
        future = std::async(std::launch::async, &Core::decompressBlock, this, block.data(), size, out_block.data());
        valid = true;
    }
}

uint32_t Core::get() {
    if (valid) {
        uint32_t res = future.get();
        valid = false;
        return res;
    }
    return 0;
}

int Core::get_cores() {
    int processor_count = (int) std::thread::hardware_concurrency();
    if (processor_count == 0) {
        processor_count = 1;
    }
    return processor_count;
}
