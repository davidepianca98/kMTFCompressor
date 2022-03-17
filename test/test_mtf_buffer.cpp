
#include <cstdint>
#include <iostream>

void mtfShift(uint64_t& buf, uint8_t c, uint8_t i) {
    // If the position is zero, no need to change the buffer
    if (i != 0) {
        int bits = (i + 1) * 8;
        uint64_t left = (buf >> bits) << bits; // Extract the part to be preserved
        bits = (8 - i) * 8;
        buf = (buf << bits) >> (bits - 8); // Make space for the character in the first position and clean the leftmost bytes
        buf |= left | c; // Put character in the first position
    }
}

void mtfAppend(uint64_t& buf, uint8_t c) {
    buf = (buf << 8) | c;
}

uint8_t mtfExtract(const uint64_t& buf, uint8_t i) {
    return static_cast<uint8_t>((buf >> (i * 8)) & 0xFF);
}

int main() {

    uint64_t t = 0x0102030405060708;
    mtfShift(t, 0x03, 5);
    std::cout << std::hex << t << std::endl;

    mtfAppend(t, 0x09);
    std::cout << std::hex << t << std::endl;

    std::cout << (uint64_t) mtfExtract(t, 1) << std::endl;
}
