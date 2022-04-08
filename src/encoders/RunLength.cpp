
#include "RunLength.h"

RunLength::RunLength(int alphabet_size, int n) : ahrle(UINT8_MAX + 1), ah(alphabet_size), n(n) {}

void RunLength::encode_array(const uint32_t *data, int length, obitstream& out) {
    if (length > 0) {
        for (int i = 0; i < length; i++) {
            if (data[i] == last) {
                counter++;
                if (counter >= 255) {
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
            last = data[i];
        }
        if (counter >= n) {
            ahrle.encode(counter - n, out);
        }
    }
    //ah.normalizeWeights();
}

void RunLength::encode(uint32_t symbol, obitstream& out) {
    ah.encode(symbol, out);
}

int RunLength::decode_array(ibitstream& in, uint32_t *data, int length, uint32_t eof) {
    int i = 0;
    while (i < length) {
        int num = ah.decode(in);
        // Check if error happened or EOF symbol reached
        if (num < 0 || num == eof || !in.remaining()) {
            return i;
        } else {
            data[i++] = num;
            if (last == num) {
                counter++;
                if (counter >= n) {
                    int len = ahrle.decode(in);
                    for (int j = 0; j < len; j++) {
                        data[i++] = num;
                    }
                    counter = 0;
                }
            } else {
                counter = 1;
            }
            last = num;
        }
    }
    return i;
}
