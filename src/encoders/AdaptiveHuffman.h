
#ifndef MTF_ADAPTIVEHUFFMAN_H
#define MTF_ADAPTIVEHUFFMAN_H

#include <iostream>
#include "stream/ibitstream/ibitstream.h"
#include "stream/obitstream/obitstream.h"

class AdaptiveHuffman {
private:

    static constexpr int MAX_ALPHA_SIZE = 256 + 256 + 1;
    static constexpr int MAX_TREE_NODES = MAX_ALPHA_SIZE * 2 - 1;

    struct TreeNode {
        uint64_t weight = 0; // Number of times the symbol has been seen
        uint16_t symbol = 0; // If is alphabet_size, this is the NYT node

        int16_t parent = -1;
        int16_t left = -1;
        int16_t right = -1;
    };

    TreeNode tree[MAX_TREE_NODES];
    int16_t nyt_node;
    int16_t next_free_slot;
    uint32_t alphabet_size;
    int log_alphabet_size;

    int16_t map_leaf[MAX_ALPHA_SIZE] = { 0 }; // gets the leaf representing the symbol, indexed by symbol
    uint64_t map_code[MAX_ALPHA_SIZE] = { 0 }; // gets the code and its length representing the symbol, indexed by symbol
    int16_t map_code_length[MAX_ALPHA_SIZE] = { 0 }; // gets the code and its length representing the symbol, indexed by symbol

    bool eof;

    void invalidate_cache();

    inline bool is_leaf(int node) {
        return tree[node].left == -1 && tree[node].right == -1;
    }

    void swap(int16_t& first, int16_t& second);

    static void write_symbol(uint64_t bits, int length, obitstream& out);

    void write_symbol(int node, obitstream& out);

    int16_t get_block_leader(int16_t node);

    void slide_and_increment(int16_t node);

    void update_tree(uint32_t symbol);

public:
    explicit AdaptiveHuffman(uint32_t alphabet_size);

    void encode(uint32_t symbol, obitstream& out);

    void encode_array(const uint32_t *data, int length, obitstream& out);

    void encode_end(uint32_t eof_symbol, obitstream& out);

    int decode(ibitstream& in);

    int decode_array(ibitstream& in, uint32_t *data, int length, uint32_t eof_symbol);

    void normalize_weights();

    uint64_t refresh_internal_weights(int node);

    void printBT(const std::string& prefix, int node, bool isRight) {
        if (node != -1) {
            std::cout << prefix;

            std::cout << (isRight ? "├──" : "└──" );

            // print the value of the node
            if (is_leaf(node)) {
                if (tree[node].symbol == alphabet_size) {
                    std::cout << "NYT" << std::endl;
                } else {
                    std::cout << (char) tree[node].symbol << std::endl;
                }
            } else {
                std::cout << tree[node].weight << std::endl;
            }

            printBT(prefix + (isRight ? "│   " : "    "), tree[node].right, true);
            printBT(prefix + (isRight ? "│   " : "    "), tree[node].left, false);
        }
    }

    void printBT(int node) {
        printBT("", node, false);
    }

    void print_tree() {
        printBT(0);
    }
};


#endif //MTF_ADAPTIVEHUFFMAN_H
