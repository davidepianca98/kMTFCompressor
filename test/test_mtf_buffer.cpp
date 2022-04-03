
#include <cstdint>
#include <iostream>
#include <vector>
#include "MTFBuffer.h"
#include "MTFRankBuffer.h"


int main() {
    MTFBuffer<uint64_t> buf;
    buf.append(0x01);
    buf.append(0x02);
    buf.append(0x03);
    buf.append(0x04);
    buf.append(0x05);
    buf.append(0x06);
    buf.append(0x07);
    buf.append(0x08);
    buf.shift(0x03, 5);
    std::cout << std::hex << buf.get_buf() << std::endl;

    buf.append(0x09);
    std::cout << std::hex << buf.get_buf() << std::endl;

    std::cout << (uint64_t) buf.extract(1) << std::endl << std::endl;


    MTFRankBuffer<uint64_t> buf2;
    buf2.append(0x08);
    buf2.append(0x07);
    buf2.append(0x06);
    buf2.append(0x05);
    buf2.append(0x04);
    buf2.append(0x03);
    buf2.append(0x02);
    buf2.append(0x01);

    buf2.shift(0x08, 0);
    buf2.shift(0x08, 0);
    buf2.shift(0x07, 1);
    std::cout << std::hex << buf2.get_buf() << std::endl;

    buf2.shift(0x03, 5);
    std::cout << std::hex << buf2.get_buf() << std::endl;

    buf2.append(0x09);
    std::cout << std::hex << buf2.get_buf() << std::endl;

    /*uint64_t t = 0x0000030405060708;
    count[6] = 0;
    count[7] = 0;
    mtfAppendRank(t, count, 0x09);
    std::cout << std::hex << t << std::endl;

    std::cout << (uint64_t) mtfExtract(t, 7) << std::endl;*/
}
