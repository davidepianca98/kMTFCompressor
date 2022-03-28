
#ifndef MTF_FASTPFORENCODER_H
#define MTF_FASTPFORENCODER_H

#include <fastpfor/codecs.h>
#include <fastpfor/codecfactory.h>

class FastPForEncoder {
public:
    static uint32_t compress(const uint32_t *block, size_t size, uint32_t *out_block) {
        using namespace FastPForLib;

        uint64_t compressed_size = size * 4 + 1024;

        // simdoptpfor, simdfastpfor256 are the best
        IntegerCODEC *codec = new CompositeCodec<SIMDOPTPFor<4, Simple16 <false>>, VariableByte>();
        //IntegerCODEC *codec = new CompositeCodec<SIMDFastPFor<8>, VariableByte>();

        codec->encodeArray(block, size, out_block, compressed_size);
        compressed_size *= 4;
        delete codec;

        return compressed_size;
    }

    static uint32_t decompress(const uint32_t *data, size_t size, uint32_t *out_block) {
        using namespace FastPForLib;

        uint64_t decompressed_size = data[0] * 4 * 32 + 1024;

        IntegerCODEC *codec = new CompositeCodec<SIMDOPTPFor<4, Simple16<false>>, VariableByte>();

        codec->decodeArray(data, size, out_block, decompressed_size);
        delete codec;

        return decompressed_size;
    }
};

#endif //MTF_FASTPFORENCODER_H
