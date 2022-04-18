
#ifndef MTF_MTFHASHCONTEXT_H
#define MTF_MTFHASHCONTEXT_H

#include <cstdint>
#include <future>
#include "stream/obitstream/obitstream.h"
#include "MTFHashTable.h"
#include "encoders/RunLength.h"

template <typename HASH, uint32_t SIZE>
class MTFHashContext {

    std::vector<uint8_t> byte_array;
    std::vector<uint32_t> int_array;

    static constexpr uint32_t CONTEXT_UP = 256 + SIZE;
    static constexpr uint32_t CONTEXT_DOWN = 256 + SIZE + 1;

    int context = 1;
    MTFHashTable<HASH, SIZE> *tables[6];
    uint64_t block_size;
    int k;
    int cur_k = 0;

    uint64_t symbols_out[256 + SIZE + 2] = { 0 };

public:
    MTFHashContext(int block_size, uint64_t max_memory_usage, int k, uint64_t seed) : block_size(block_size), k(k) {
        byte_array.resize(block_size * 3);
        int_array.resize(block_size * 3);

        context = k;

        for (int i = 0; i <= k; i++) {
            tables[i] = new MTFHashTable<HASH, SIZE>(block_size, max_memory_usage, i, seed);
        }
    }

    ~MTFHashContext() {
        for (int i = cur_k; i <= k; i++) {
            delete tables[i];
        }
    }

    uint32_t cost(uint32_t symbol, int new_context) {
        int jumps = new_context - context;
        uint32_t cost = 0;
        //if (symbol < SIZE && symbol < min && i >= best_k) {
        if (symbol >= SIZE) {
            cost += 4;
        }
        if (jumps < 0) {
            cost += (uint32_t) (1.5 * abs(jumps));
        } else {
            cost += jumps;
        }
        return cost;
    }

    void encode(uint8_t c, std::vector<uint32_t>& out, int& out_index, bool switch_allowed) {
        uint32_t best_symbol = tables[context - 1]->mtf_encode(c);
        uint32_t min = cost(best_symbol, context);
        int best_k = context;
        for (int i = 1; i <= k; i++) {
            if (i != context) {
                uint32_t symbol = tables[i - 1]->mtf_encode(c); // TODO thread pool
                uint32_t new_cost = cost(symbol, i);
                if (new_cost < min && switch_allowed) {
                    best_symbol = symbol;
                    min = new_cost;
                    best_k = i;
                }
            }
        }

        int diff = best_k - context;
        for (int i = 0; i < abs(diff); i++) {
            if (diff > 0) {
                //std::cout << "UP\n";
                out[out_index++] = CONTEXT_UP;
                symbols_out[CONTEXT_UP]++;
            } else if (diff < 0) {
                //std::cout << "DO\n";
                out[out_index++] = CONTEXT_DOWN;
                symbols_out[CONTEXT_DOWN]++;

            }
        }
        out[out_index++] = best_symbol;
        symbols_out[best_symbol]++;

        context = best_k;
    }

    void encode(std::istream& in, obitstream& out) {
        std::future<void> future;
        auto *out_block1 = new uint32_t[block_size];

        RunLength rle(256 + SIZE + 1);
        cur_k = 0;
        uint64_t len = 32;
        uint64_t read_size = 0;
        long read_bytes;
        do {
            // Read block
            in.read(reinterpret_cast<char *>(byte_array.data()), std::min(len, block_size));
            read_bytes = in.gcount();

            // Apply transformation
            for (int j = cur_k; j <= k; j++) {
                // TODO run thread for every j with following loop, while run in current thread the j==cur_k
                for (int i = 0; i < read_bytes; i++) {
                    uint32_t symbol = tables[j]->mtf_encode(byte_array[i]);
                    if (j == cur_k) {
                        int_array[i] = symbol;
                    }
                }
            }

            if (future.valid()) {
                future.wait();
            }

            memcpy(out_block1, int_array.data(), read_bytes * 4);
            future = std::async(std::launch::async, &RunLength::encode_array, &rle, out_block1, read_bytes, std::ref(out));

            read_size += read_bytes;
            if (read_size >= len) {
                if (cur_k < k) {
                    len *= 32;
                    delete tables[cur_k];
                    cur_k++;
                }
            }
        } while (read_bytes > 0);
        if (future.valid()) {
            future.wait();
        }
        rle.encode_end(256 + SIZE, out);
        out.flush_remaining();

        delete[] out_block1;
    }

    void print_stats() {
        for (uint32_t i : symbols_out) {
            std::cout << i << ",";
        }
        std::cout << std::endl;
    }
};

#endif //MTF_MTFHASHCONTEXT_H
