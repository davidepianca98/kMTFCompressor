
#ifndef MTF_ADAPTIVEHUFFMAN_H
#define MTF_ADAPTIVEHUFFMAN_H

#include "stream/ibitstream/ibitstream.h"
#include "stream/obitstream/obitstream.h"

class AdaptiveHuffman {
private:

    struct TreeNode {
        uint32_t symbol = 0;
        uint64_t weight = 0; // Number of times the symbol has been seen
        uint64_t number = 0;

        int parent = -1;
        int left = -1;
        int right = -1;

        bool nyt = false; // true if not yet transmitted node
    };

    std::vector<TreeNode> tree;
    int nyt_node;
    int next_free_slot;
    uint32_t alphabet_size;
    int log_alphabet_size;

    std::vector<int> map_leaf; // gets the leaf representing the symbol, indexed by symbol


    inline bool is_leaf(int node);

    int get_block_leader(int node);

    void swap(int& first, int& second);

    static void write_symbol(uint32_t bits, int length, obitstream& out);

    void write_symbol(int node, obitstream& out);

    void slide_and_increment(int node);

    void update_tree(uint32_t symbol);

public:
    explicit AdaptiveHuffman(uint32_t alphabet_size);

    void encode(uint32_t symbol, obitstream& out);

    void encode(const uint32_t *data, uint32_t length, obitstream& out);

    int decode(ibitstream& in);

    uint32_t decode(ibitstream& in, uint32_t *data, uint32_t length, uint32_t eof);

    void normalizeWeights();
};


#endif //MTF_ADAPTIVEHUFFMAN_H
