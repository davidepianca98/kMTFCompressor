
#include "MTFBuffer.h"

template <typename T>
void MTFBuffer<T>::shift(uint8_t c, uint8_t i) {
    // If the position is zero, no need to change the buffer
    if (i != 0) {
        int bits = (i + 1) * 8;
        T left = (buf >> bits) << bits; // Extract the part to be preserved
        bits = (byte_size() - i) * 8;
        buf = (buf << bits) >> (bits - 8); // Make space for the character in the first position and clean the leftmost bytes
        buf |= left | c; // Put character in the first position
    }
}

template <typename T>
void MTFBuffer<T>::append(uint8_t c) {
    buf = (buf << 8) | c;
    if (symbols < byte_size()) {
        symbols++;
    }
}

template <typename T>
uint8_t MTFBuffer<T>::extract(uint8_t i) {
    return static_cast<uint8_t>((buf >> (i * 8)) & 0xFF);
}

template <typename T>
uint32_t MTFBuffer<T>::encode(uint8_t c) {
    for (uint8_t i = 0; i < symbols; i++) {
        uint8_t extracted = extract(i);
        if (extracted == c) { // Check if the character in the i-th position from the right is equal to c
            shift(c, i);
            return i;
        }
    }

    // Not found so add the symbol to the buffer
    append(c);

    // Sum byte_size to differentiate between indexes on the MTF buffer and characters
    return c + byte_size();
}

template <typename T>
uint8_t MTFBuffer<T>::decode(uint32_t symbol) {
    uint8_t c;
    if (symbol >= byte_size()) {
        c = symbol - byte_size();
        append(c);
    } else {
        c = extract(symbol);
        shift(c, symbol);
    }
    return c;
}

template <typename T>
T MTFBuffer<T>::get_buf() {
    return buf;
}

template <typename T>
bool MTFBuffer<T>::visited() {
    return is_visited;
}

template <typename T>
void MTFBuffer<T>::set_visited(uint64_t hash) {
    is_visited = true;
    key = hash;
}

template<typename T>
uint64_t MTFBuffer<T>::get_key() {
    return key;
}

template class MTFBuffer<uint16_t>;
template class MTFBuffer<uint32_t>;
template class MTFBuffer<uint64_t>;
template class MTFBuffer<boost::multiprecision::uint128_t>;
template class MTFBuffer<boost::multiprecision::uint256_t>;
template class MTFBuffer<boost::multiprecision::uint512_t>;
template class MTFBuffer<boost::multiprecision::uint1024_t>;
