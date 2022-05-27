
#include <iostream>
#include <cassert>
#include "RunLength.h"

RunLength::RunLength(int alphabet_size, int n) : ahrle(256), ah(alphabet_size), n(n), counter(0), last(-1), remaining(0), eof(false) {}

void RunLength::encode_array(const uint32_t *data, int length, obitstream& out) {
    for (int i = 0; i < length; i++) {
        if ((int) data[i] == last) {
            counter++;
            if (counter >= 255 + n) {
                ahrle.encode(counter - n, out);
                counter = 0;
            } else if (counter <= n) {
                ah.encode(last, out);
            }
        } else {
            if (counter >= n) {
                ahrle.encode(counter - n, out);
            }
            ah.encode(data[i], out);
            counter = 1;
        }
        last = (int) data[i];
    }
    //ah.normalize_weights();
}

void RunLength::encode_end(uint32_t eof_symbol, obitstream& out) {
    if (counter >= n) {
        ahrle.encode(counter - n, out);
        counter = 0;
    }
    ah.encode(eof_symbol, out);
    eof = true;
}

int RunLength::decode_array(ibitstream& in, uint32_t *data, int length, uint32_t eof_symbol) {
    if (eof) {
        return 0;
    }
    int i = 0;
    while (remaining > 0 && i < length) {
        data[i++] = last;
        remaining--;
    }
    while (i < length) {
        int num = ah.decode(in);
        // Check if error happened or EOF symbol reached
        if (num < 0 || (uint32_t) num == eof_symbol || !in.remaining()) {
            eof = true;
            assert(i <= length);
            return i;
        }
        data[i++] = num;
        if (last == num) {
            counter++;
        } else {
            counter = 1;
        }
        if (counter >= n) {
            counter = 0;
            int len = ahrle.decode(in);
            if (len < 0) {
                eof = true;
                return i;
            }
            for (int j = 0; j < len; j++) {
                if (i >= length) {
                    remaining = len - j;
                    last = num;
                    return i;
                }
                data[i++] = num;
            }
        }
        last = num;
    }
    assert(i <= length);
    return i;
}
