
#ifndef MTF_SIMHASH_H
#define MTF_SIMHASH_H


#include <vector>
#include <cmath>
#include <ctime>
#include "Hash.h"

class SimHash : public Hash {
private:
    std::vector<std::vector<uint8_t>> vectors;

    void compute() {
        uint64_t result = 0;
        for (int i = 0; i < vectors.size(); i++) {
            // dot(a, rot90CCW(b))
            // rot90CCW(b) = {x: -b.y, y: b.x}
            uint32_t bit = ((kmer[0] * -vectors[i][1]) + (kmer[1] * vectors[i][0])) >= 0; // if > 0, v is on the right of kmer
            result |= bit << i;
        }

        hash = result;
    }

public:

    explicit SimHash(int k, int size) : Hash(k, k, size) {}

    void update(uint8_t c) override {
        for (int i = 0; i < kmer.size() - 1; i++) {
            kmer[i] = kmer[i + 1];
        }
        kmer[kmer.size() - 1] = c;
        compute();
    }

    void init(const std::vector<uint8_t> &start) override {
        srand(time(nullptr)); // TODO should be generated once and then hardcoded

        for (int i = 0; i < k; i++) {
            kmer[i] = start[i];
        }

        int bit_count = (int) log2(size);// + 1;
        int angle = 90 / (bit_count + 1); // Ignore axes

        // Along x axis
        std::vector<uint8_t> ref(k);
        ref[0] = 100;
        ref[1] = 0;

        for (int i = 0; i < bit_count; i++) {
            std::vector<uint8_t> v(k);

            int rot = (i + 1) * angle;

            // Rotate ref
            v[0] = ref[0] * cos(rot) - ref[1] * sin(rot);
            v[1] = ref[0] * sin(rot) + ref[1] * cos(rot);

            vectors.emplace_back(v);
        }
        compute();
    }

    void resize(uint64_t size) override {
        this->size = size;
    }
};


#endif //MTF_SIMHASH_H
