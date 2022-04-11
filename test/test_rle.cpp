
#include <cstring>
#include "encoders/RunLength.h"
#include "stream/obitstream/obufbitstream.h"
#include "stream/ibitstream/ibufbitstream.h"

int main() {
    uint32_t data[] = { 1, 1, 1, 3, 4, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 6, 2, 2 };
    static constexpr int len = sizeof(data) / sizeof(uint32_t);
    uint8_t out_buf[len];
    uint32_t out_buf2[len];

    RunLength rle1(10, 4);
    obufbitstream out(out_buf, len);
    rle1.encode_array(data, len, out);
    rle1.encode_end(9, out);
    out.flush_remaining();

    RunLength rle2(10, 4);
    ibufbitstream in(out_buf, len);
    rle2.decode_array(in, out_buf2, len, 9);

    return memcmp(data, out_buf2, len * 4);
}
