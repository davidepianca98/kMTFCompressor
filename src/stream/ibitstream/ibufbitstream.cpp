
#include "ibufbitstream.h"

ibufbitstream::ibufbitstream(uint8_t *buffer, int size) {
    sb.pubsetbuf(reinterpret_cast<char *>(buffer), size);
    init(&sb);
}
