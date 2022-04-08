
#include <cstdint>
#include "obufbitstream.h"

obufbitstream::obufbitstream(uint8_t *buffer, int size) {
    sb.pubsetbuf(reinterpret_cast<char *>(buffer), size);
    init(&sb);
}

uint64_t obufbitstream::size() {
    if (obitstream::bit_count % 8 > 0) {
        return obitstream::bit_count / 8 + 1;
    }
    return obitstream::bit_count / 8;
}
