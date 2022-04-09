
#include <cmath>
#include <cstdint>
#include <cassert>
#include "AdaptiveHuffman.h"

AdaptiveHuffman::AdaptiveHuffman(uint32_t alphabet_size): tree(alphabet_size * 2 + 1), nyt_node(0), next_free_slot(1),
                                                          alphabet_size(alphabet_size), map_leaf(alphabet_size, -1) {
    tree[0].symbol = alphabet_size;
    tree[0].number = alphabet_size * 2;
    tree[0].nyt = true;

    log_alphabet_size = (int) log2(alphabet_size);
}

inline bool AdaptiveHuffman::is_leaf(int node) {
    return tree[node].left == -1 && tree[node].right == -1;
}

int AdaptiveHuffman::get_block_leader(int node) {
    // Find the highest number node in nodes of same weight, the nodes are ordered by decreasing number and weight
    int leader = node;
    for (int i = node - 1; i >= 0 && tree[i].weight == tree[leader].weight; i--) {
        if (i != tree[leader].parent) {
            leader = i;
        }
    }

    return leader;
}

void AdaptiveHuffman::swap(int& first, int& second) {
    int first_parent = tree[first].parent;
    int second_parent = tree[second].parent;
    if (first_parent == -1 || second_parent == -1 || first_parent == second || second_parent == first) {
        return;
    }

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
    std::swap(tree[first].number, tree[second].number);
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

void AdaptiveHuffman::write_symbol(uint32_t bits, int length, obitstream& out) {
    for (int i = 0; i < length; i++) {
        out.write_bit((bits >> i) & 1);
    }
}

void AdaptiveHuffman::write_symbol(int node, obitstream& out) {
    uint32_t bits = 0; // List of bits that represent the symbol
    int n = 0;
    // Traverse the tree from the leaf until the node before the root is reached
    while (node != -1 && tree[node].parent != -1) {
        int parent = tree[node].parent;
        bits <<= 1;
        // Append 1 if we came from the right, otherwise leave it as 0
        if (tree[parent].right == node) {
            bits |= 1;
        }
        n++;
        node = parent;
    }

    // Write out the bits in the opposite order, because they need to be from root to leaf
    write_symbol(bits, n, out);
}

void AdaptiveHuffman::slide_and_increment(int node) {
    while (node != -1) {
        int leader = get_block_leader(node);
        if (leader != node) {
            swap(leader, node);
        }

        tree[node].weight++;
        node = tree[node].parent;
    }
}

void AdaptiveHuffman::update_tree(uint32_t symbol) {
    if (map_leaf[symbol] == -1) {
        // First time the symbol has been seen, split the NYT node in a new NYT node and the symbol node
        tree[nyt_node].nyt = false;
        uint64_t number = tree[nyt_node].number;

        tree[nyt_node].right = next_free_slot;
        tree[next_free_slot].symbol = symbol;
        tree[next_free_slot].number = number - 1;
        tree[next_free_slot].parent = nyt_node;
        tree[next_free_slot].nyt = false;

        next_free_slot++;

        tree[nyt_node].left = next_free_slot;
        tree[next_free_slot].symbol = alphabet_size;
        tree[next_free_slot].number = number - 2;
        tree[next_free_slot].parent = nyt_node;
        tree[next_free_slot].nyt = true;

        next_free_slot++;

        map_leaf[symbol] = tree[nyt_node].right;
        nyt_node = tree[nyt_node].left;
    }

    slide_and_increment(map_leaf[symbol]);
}

void AdaptiveHuffman::encode(uint32_t symbol, obitstream& out) {
    assert(symbol < alphabet_size);
    if (map_leaf[symbol] != -1) {
        write_symbol(map_leaf[symbol], out);
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

void AdaptiveHuffman::encode(const uint32_t *data, uint32_t length, obitstream& out) {
    for (uint32_t i = 0; i < length; i++) {
        encode(data[i], out);
    }
}

int AdaptiveHuffman::decode(ibitstream& in) {
    int node = 0;

    // Traverse the tree until a leaf is reached
    while (!is_leaf(node)) {
        int bit = in.read_bit();
        if (bit == 0) {
            node = tree[node].left;
        } else if (bit == 1) {
            node = tree[node].right;
        } else {
            return -1;
        }
    }

    uint32_t number = 0;
    if (tree[node].nyt) {
        // If the leaf is the NYT node, read log_2(alphabet_size) + 1 bits
        for (int i = log_alphabet_size; i >= 0; i--) {
            number <<= 1;
            int bit = in.read_bit();
            if (bit == -1) {
                return -1;
            }
            number |= bit;
        }
    } else {
        number = tree[node].symbol;
    }
    if (number >= alphabet_size) {
        return -1;
    }
    update_tree(number);
    return (int) number;
}

uint32_t AdaptiveHuffman::decode(ibitstream& in, uint32_t *data, uint32_t length, uint32_t eof) {
    uint32_t decompressed_size = 0;
    while (decompressed_size < length) {
        int value = decode(in);
        if (value == -1) {
            break;
        }
        data[decompressed_size] = value;
        if (data[decompressed_size] == eof) {
            break;
        }
        decompressed_size++;
    }
    return decompressed_size;
}

void AdaptiveHuffman::normalizeWeights() {
    for (auto & node : tree) {
        node.weight = (uint64_t) log2((double) (node.weight + 1)); // TODO this generates infinite loop in write_symbol in big files, probably doesn't preserve the sum of children weights in parent weight, so apply log to leaves and then set internal nodes weights
    }
}
