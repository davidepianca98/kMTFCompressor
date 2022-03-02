
#include <future>
#include <thread>
#include <queue>
#include "MTFHashTable.h"
#include "hash/Hash.h"
#include "hash/RabinFingerprint.h"
#include "hash/DumbHash.h"
#include "hash/MinimiserHash.h"
#include "VectorHash.h"
#include "SimHash.h"
#include "Core.h"
#include "SPSCQueue.h"


void MTFHashTable::mtfShift(uint64_t& buf, uint8_t c, uint8_t i) {
    // If the position is zero, no need to change the buffer
    if (i != 0) {
        uint64_t left = (buf >> ((i + 1) * 8)) << ((i + 1) * 8); // Extract the part to be preserved
        buf = (buf << (8 - i) * 8) >> ((8 - i - 1) * 8); // Make space for the character in the first position and clean the leftmost bytes
        buf |= left | c; // Put character in the first position
    }
}

void MTFHashTable::mtfAppend(uint64_t& buf, uint8_t c) {
    buf = (buf << 8) | c;
}

uint8_t MTFHashTable::mtfExtract(uint64_t buf, uint8_t i) {
    return (buf >> (i * 8)) & 0xFF;
}

/*
 * MTF buffer containing at most 8 chars
 *
 * When called on a 64-bits buffer buf (8 chars, top char in least significant position) and character c,
 * looks for the rightmost occurrence of c in buf, moving it to the front and returning its position in the buffer
 * before moving c. If c is not found in buf, then c is returned.
 */
uint32_t MTFHashTable::mtfEncode(uint64_t& buf, uint8_t c) {
    for (uint8_t i = 0; i < 8; i++) {
        if (mtfExtract(buf, i) == c) { // Check if the character in the i-th position from the right is equal to c
            mtfShift(buf, c, i);
            return i;
        }
    }

    // Not found so shift left and put character in first position
    mtfAppend(buf, c);

    // Sum 8 to differentiate between indexes on the MTF buffer and characters
    return c + 8;
}

uint8_t MTFHashTable::mtfDecode(uint64_t& buf, uint32_t i) {
    if (i >= 8) {
        mtfAppend(buf, i - 8);
        return i - 8;
    } else {
        uint8_t ca = mtfExtract(buf, i);
        mtfShift(buf, ca, i);
        return ca;
    }
}

void MTFHashTable::keep_track(uint64_t hash) {
    if (!visited[hash]) {
        used_cells += 1;
        visited[hash] = true;
    }
}

// TODO size chosen based on the hash
MTFHashTable::MTFHashTable(int k): hash_table(10007, 0), visited(10007, false), k(k) {}

void MTFHashTable::print_stats() const {
    std::cout << "Used hash cells = " << used_cells << std::endl;
    std::cout << "Hash table load = " << used_cells / double(hash_table.size()) << std::endl;
}

void MTFHashTable::encode(const uint8_t *block, long size, uint32_t *out_block) {
    uint8_t c;

    std::vector<uint8_t> start(k);
    int i;
    for (i = 0; i < k; i++) {
        c = block[i];
        start[i] = c;
        out_block[i] = (uint32_t) c + 8;
    }

    // Rolling hash to access the table
    RabinFingerprint hash(k, start);

    while (i < size) {
        c = block[i];
        keep_track(hash.get_hash());

        uint32_t pos = mtfEncode(hash_table[hash.get_hash()], c);
        out_block[i] = pos;
        hash.update(c);
        i++;
    }
}

void MTFHashTable::encode2(std::istream& in, std::ostream& out) {
    uint8_t c;

    std::vector<uint8_t> start(k);
    for (int i = 0; i < k; i++) {
        in >> c;
        start[i] = c;
        uint32_t res = (uint32_t) c + 8;
        out << res;
    }

    // Rolling hash to access the table
    RabinFingerprint hash(k, start);

    int num_cores = Core::get_cores() - 1;
    std::vector<std::future<uint32_t>> futures;

    while (in.good()) {
        for (int i = 0; i < num_cores; i++) {
            in >> c;
            if (!in.eof()) {
                keep_track(hash.get_hash());

                // TODO assign to thread depending on hash value in range
                auto f = std::async(std::launch::async, [this](uint32_t hash, uint8_t c) {
                    return mtfEncode(hash_table[hash], c);
                }, hash.get_hash(), c);
                futures.push_back(std::move(f));

                hash.update(c);
            }
        }

        for (auto& future : futures) {
            uint32_t res = future.get();
            out << res;
        }
        futures.clear();
    }
}

std::atomic_bool stop_thread = false;

void MTFHashTable::thread_f(rigtorp::SPSCQueue<std::tuple<uint8_t, uint64_t, uint32_t *>> *q) {
    while (!stop_thread || !q->empty()) {
        std::tuple<uint8_t, uint64_t, uint32_t *> *p = q->front();
        if (p) {
            q->pop();
            uint8_t c = std::get<0>(*p);
            uint64_t hash = std::get<1>(*p);
            uint32_t *out = std::get<2>(*p);
            uint32_t pos = mtfEncode(hash_table[hash], c);
            *out = pos;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1)); // TODO should lock and wait
        }
    }
}

void MTFHashTable::encode3(std::istream& in, std::ostream& out) {
    uint8_t c;
    stop_thread = false;

    int num_cores = 1;//Core::get_cores() - 2;

    auto **queues = new rigtorp::SPSCQueue<std::tuple<uint8_t, uint64_t, uint32_t *>>*[num_cores];
    std::vector<std::thread> threads;
    for (int i = 0; i < num_cores; i++) {
        queues[i] = new rigtorp::SPSCQueue<std::tuple<uint8_t, uint64_t, uint32_t *>>(100);
        threads.emplace_back(&MTFHashTable::thread_f, this, queues[i]);
    }

    int block_size = 1024 * 1024; // 1 MB block size
    uint8_t in_data[block_size];
    uint32_t mtf_out_data[block_size];
    uint8_t *out_data = new uint8_t[block_size * 4 + 1024];



    std::vector<uint8_t> start(k);
    int i;
    for (i = 0; i < k; i++) {
        in.read(reinterpret_cast<char *>(&c), 1);
        in_data[i] = c;
        start[i] = c;
        mtf_out_data[i] = (uint32_t) c + 8;
    }

    // Rolling hash to access the table
    RabinFingerprint hash(k, start);

    double divider = (double) num_cores / hash_table.size();

    while (in.good()) {
        in.read(reinterpret_cast<char *>(in_data + i), block_size - i);
        long read_bytes = in.gcount() + i;

        while (i < block_size) {
            c = in_data[i];
            keep_track(hash.get_hash());

            //std::tuple<uint8_t, uint64_t, uint32_t *> p(c, hash.get_hash(), &mtf_out_data[i]);
            //int thread_number = hash.get_hash() * divider; // TODO check distribution of thread number
            //queues[thread_number]->push(p);
            mtf_out_data[i] = mtfEncode(hash_table[hash.get_hash()], c);
            //std::cout << mtf_out_data[i] << " ";

            hash.update(c);
            i++;
        }
        i = 0;

        for (int j = 0; j < num_cores; j++) {
            while (!queues[j]->empty()) { // TODO not really correct
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        size_t compressed_size = read_bytes * 4 + 1024;
        // TODO maybe spawn other thread to parallelize con future
        Core::compress_final(mtf_out_data, read_bytes, reinterpret_cast<uint32_t *>(out_data), compressed_size);

        out.write(reinterpret_cast<const char *>(&compressed_size), 4);
        out.write(reinterpret_cast<const char *>(out_data), compressed_size);

    }


    stop_thread = true;
    for (i = 0; i < num_cores; i++) {
        threads[i].join();
        delete queues[i];
    }
    delete[] queues;
    delete[] out_data;
}

void MTFHashTable::decode(const uint32_t *block, long size, uint8_t *out_block) {
    uint32_t c;

    std::vector<uint8_t> start(k);
    int i;
    for (i = 0; i < k; i++) {
        c = block[i];
        start[i] = (uint8_t) (c - 8);
        out_block[i] = start[i];
    }

    // Rolling hash to access the table
    RabinFingerprint hash(k, start);

    while (i < size) {
        c = block[i];
        uint8_t ca = mtfDecode(hash_table[hash.get_hash()], c);
        out_block[i] = ca;

        hash.update(ca);
        i++;
    }
}
