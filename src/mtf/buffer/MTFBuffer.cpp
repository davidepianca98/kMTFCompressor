
#include "MTFBuffer.h"

template <uint32_t SIZE>
void MTFBuffer<SIZE>::shift(uint8_t c, uint8_t i) {
    // If the position is zero, no need to change the buffer
    for (int j = i; j > 0; j--) {
        buffer[j] = buffer[j - 1];
    }
    buffer[0] = c;
}

template <uint32_t SIZE>
void MTFBuffer<SIZE>::append(uint8_t c) {
    if (symbols >= SIZE) {
        symbols--;
    }
    for (int j = symbols; j > 0; j--) {
        buffer[j] = buffer[j - 1];
    }
    buffer[0] = c;
    symbols++;
}

template <uint32_t SIZE>
uint8_t MTFBuffer<SIZE>::extract(uint8_t i) {
    return buffer[i];
}

template <uint32_t SIZE>
uint32_t MTFBuffer<SIZE>::encode(uint8_t c) {
    for (uint8_t i = 0; i < symbols; i++) {
        uint8_t extracted = extract(i);
        if (extracted == c) { // Check if the character in the i-th position from the right is equal to c
            shift(c, i);
            return i;
        }
    }

    // Not found so add the symbol to the buffer
    append(c);

    // Sum size to differentiate between indexes on the MTF buffer and characters
    return c + SIZE;
}

template <uint32_t SIZE>
uint8_t MTFBuffer<SIZE>::decode(uint32_t symbol) {
    uint8_t c;
    if (symbol >= SIZE) {
        c = symbol - SIZE;
        append(c);
    } else {
        c = extract(symbol);
        shift(c, symbol);
    }
    return c;
}

template <uint32_t SIZE>
bool MTFBuffer<SIZE>::visited() {
    return is_visited;
}

template <uint32_t SIZE>
void MTFBuffer<SIZE>::set_visited(uint64_t hash) {
    is_visited = true;
    key = hash;
}

template<uint32_t SIZE>
uint64_t MTFBuffer<SIZE>::get_key() {
    return key;
}

template class MTFBuffer<2>;
template class MTFBuffer<4>;
template class MTFBuffer<6>;
template class MTFBuffer<8>;
template class MTFBuffer<16>;
template class MTFBuffer<32>;
template class MTFBuffer<64>;
template class MTFBuffer<128>;
template class MTFBuffer<256>;
