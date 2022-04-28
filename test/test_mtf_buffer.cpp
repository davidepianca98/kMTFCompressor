
#include <iostream>
#include <vector>
#include "mtf/buffer/MTFBuffer.h"
#include "mtf/buffer/CountBuffer.h"


int main() {
    MTFBuffer<8> buf;
    buf.append(1);
    buf.append(2);
    buf.append(3);
    buf.append(4);
    buf.append(5);
    buf.append(6);
    buf.append(7);
    buf.append(8);
    buf.shift(5);
    if (buf.extract(0) != 3) {
        return 1;
    }

    buf.append(9);
    if (buf.extract(0) != 9) {
        return 1;
    }
    if (buf.extract(1) != 3) {
        return 1;
    }


    CountBuffer<8> buf2;
    buf2.append(8);
    buf2.append(7);
    buf2.append(6);
    buf2.append(5);
    buf2.append(4);
    buf2.append(3);
    buf2.append(2);
    buf2.append(1);

    buf2.shift(0);
    buf2.shift(0);
    buf2.shift(1);
    if (buf2.extract(0) != 8) {
        return 1;
    }
    if (buf2.extract(1) != 7) {
        return 1;
    }

    buf2.shift(5);
    if (buf2.extract(2) != 3) {
        return 1;
    }

    buf2.append(9);
    if (buf2.extract(7) != 9) {
        return 1;
    }

    return 0;
}
