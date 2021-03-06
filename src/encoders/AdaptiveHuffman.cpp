
#include <cmath>
#include <cstdint>
#include <cassert>
#include <cstring>
#include "AdaptiveHuffman.h"

AdaptiveHuffman::AdaptiveHuffman(uint32_t alphabet_size): nyt_node(0), next_free_slot(1), alphabet_size(alphabet_size),
                                                          eof(false) {
    assert(alphabet_size <= MAX_ALPHA_SIZE);
    tree[nyt_node].symbol = alphabet_size;

    memset(map_leaf, 0xFF, MAX_ALPHA_SIZE * 2);

    invalidate_cache();

    log_alphabet_size = (int) log2(alphabet_size);
}

void AdaptiveHuffman::invalidate_cache() {
    // Sets all the bits to 1 which is equivalent to setting every 16 bit int to -1
    memset(map_code_length, 0xFF, MAX_ALPHA_SIZE * 2);
}

void AdaptiveHuffman::swap(int16_t& first, int16_t& second) {
    assert(first >= 0 && first < next_free_slot);
    assert(second >= 0 && second < next_free_slot);
    assert(first != second);

    // Assign children parents to the other parent's index as they are about to be swapped
    if (tree[first].left != -1) {
        tree[tree[first].left].parent = second;
    }
    if (tree[first].right != -1) {
        tree[tree[first].right].parent = second;
    }
    if (tree[second].left != -1) {
        tree[tree[second].left].parent = first;
    }
    if (tree[second].right != -1) {
        tree[tree[second].right].parent = first;
    }

    // Swap the nodes but keep the same parent and order number
    std::swap(tree[first], tree[second]);
    std::swap(tree[first].parent, tree[second].parent);
    // Swap indexes of the caller
    std::swap(first, second);

    // Assign new node index to the reverse map by symbol
    if (tree[first].symbol != alphabet_size) {
        map_leaf[tree[first].symbol] = first;
    }
    if (tree[second].symbol != alphabet_size) {
        map_leaf[tree[second].symbol] = second;
    }
}

void AdaptiveHuffman::write_symbol(uint64_t bits, int length, obitstream& out) {
    assert(length >= 0 && length <= 64);

    for (int i = 0; i < length; i++) {
        out.write_bit((bits >> i) & 1);
    }
}

void AdaptiveHuffman::write_symbol(int node, obitstream& out) {
    assert(node >= 0 && node < next_free_slot);

    uint32_t symbol = tree[node].symbol;

    uint64_t bits = 0; // List of bits that represent the symbol
    int16_t n = 0;
    // Traverse the tree from the leaf until the node before the root is reached
    int parent = tree[node].parent;
    while (parent != -1) {
        bits <<= 1;
        // Append 1 if we came from the right, otherwise leave it as 0
        if (tree[parent].right == node) {
            bits |= 1;
        }
        n++;
        node = parent;
        parent = tree[node].parent;
    }

    // Write out the bits in the opposite order, because they need to be from root to leaf
    write_symbol(bits, n, out);

    map_code[symbol] = bits;
    map_code_length[symbol] = n;
}

int16_t AdaptiveHuffman::get_block_leader(int16_t node) {
    assert(node >= 0 && node < next_free_slot);

    // Find the highest number node in nodes of same weight, the nodes are ordered by decreasing number and weight
    int16_t leader = node;
    // This loop takes constant time on average
    for (int16_t i = node - 1; i >= 0 && tree[i].weight == tree[leader].weight; i--) {
        if (i != tree[leader].parent) {
            leader = i;
        }
    }

    return leader;
}

void AdaptiveHuffman::slide_and_increment(int16_t node) {
    assert(node >= -1 && node < next_free_slot);
    bool swapped = false;
    while (node != -1) {
        int16_t leader = get_block_leader(node);
        if (leader != node) {
            int first_parent = tree[leader].parent;
            int second_parent = tree[node].parent;
            // Don't swap if one of the nodes is the root and if parents aren't the other node
            if (first_parent != -1 && second_parent != -1 && first_parent != node && second_parent != leader) {
                swap(leader, node);
                swapped = true;
            }
        }

        tree[node].weight++;
        node = tree[node].parent;
    }
    if (swapped) {
        invalidate_cache();
    }
}

void AdaptiveHuffman::update_tree(uint32_t symbol) {
    assert(symbol < alphabet_size);

    if (map_leaf[symbol] == -1) {
        // First time the symbol has been seen, split the NYT node in a new NYT node and the symbol node
        tree[nyt_node].symbol = alphabet_size + 1;

        tree[nyt_node].right = next_free_slot;
        tree[next_free_slot].symbol = symbol;
        tree[next_free_slot].parent = nyt_node;

        next_free_slot++;

        tree[nyt_node].left = next_free_slot;
        tree[next_free_slot].symbol = alphabet_size;
        tree[next_free_slot].parent = nyt_node;

        next_free_slot++;

        map_leaf[symbol] = tree[nyt_node].right;
        nyt_node = tree[nyt_node].left;
    }

    slide_and_increment(map_leaf[symbol]);
}

void AdaptiveHuffman::encode(uint32_t symbol, obitstream& out) {
    assert(symbol < alphabet_size);

    if (map_leaf[symbol] != -1) {
        if (map_code_length[symbol] != -1) {
            write_symbol(map_code[symbol], map_code_length[symbol], out);
        } else {
            write_symbol(map_leaf[symbol], out);
        }
    } else {
        // If the symbol hasn't been seen yet, write the NYT escape sequence
        write_symbol(nyt_node, out);

        // Write the log_2(alphabet_size) + 1 bits fixed representation of the symbol
        for (int i = log_alphabet_size; i >= 0; i--) {
            out.write_bit((symbol >> i) & 1);
        }
    }
    update_tree(symbol);
}

void AdaptiveHuffman::encode_array(const uint32_t *data, int length, obitstream& out) {
    for (int i = 0; i < length; i++) {
        encode(data[i], out);
    }
}

void AdaptiveHuffman::encode_end(uint32_t eof_symbol, obitstream& out) {
    assert(eof_symbol < alphabet_size);
    encode(eof_symbol, out);
    eof = true;
}

int AdaptiveHuffman::decode(ibitstream& in) {
    if (eof) {
        return -1;
    }
    int node = 0;

    // Traverse the tree until a leaf is reached
    while (!is_leaf(node)) {
        int bit = in.read_bit();
        if (bit == 0) {
            node = tree[node].left;
        } else if (bit == 1) {
            node = tree[node].right;
        } else {
            eof = true;
            return -1;
        }
    }

    uint32_t number = 0;
    if (tree[node].symbol == alphabet_size) {
        // If the leaf is the NYT node, read log_2(alphabet_size) + 1 bits
        for (int i = log_alphabet_size; i >= 0; i--) {
            number <<= 1;
            int bit = in.read_bit();
            if (bit == -1) {
                eof = true;
                return -1;
            }
            number |= bit;
        }
    } else {
        number = tree[node].symbol;
    }
    assert(number < alphabet_size);
    update_tree(number);
    return (int) number;
}

int AdaptiveHuffman::decode_array(ibitstream& in, uint32_t *data, int length, uint32_t eof_symbol) {
    if (eof) {
        return 0;
    }
    int decompressed_size = 0;
    while (decompressed_size < length) {
        int value = decode(in);
        if (value == -1 || value == (int) eof_symbol) {
            eof = true;
            return decompressed_size;
        }
        data[decompressed_size++] = value;
    }
    return decompressed_size;
}

void AdaptiveHuffman::normalize_weights() {
    for (auto leaf : map_leaf) {
        if (leaf != -1) {
            tree[leaf].weight = (uint64_t) log2((double) (tree[leaf].weight + 1));
        }
    }
    refresh_internal_weights(0);
}

uint64_t AdaptiveHuffman::refresh_internal_weights(int node) {
    if (node == -1) {
        return 0;
    } else if (is_leaf(node)) {
        return tree[node].weight;
    } else {
        uint64_t left_weight = refresh_internal_weights(tree[node].left);
        uint64_t right_weight = refresh_internal_weights(tree[node].right);
        tree[node].weight = left_weight + right_weight;
        return tree[node].weight;
    }
}
