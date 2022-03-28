
#ifndef MTF_ADAPTIVEHUFFMAN_H
#define MTF_ADAPTIVEHUFFMAN_H

#include "stream/ibitstream.h"
#include "stream/obitstream.h"

class AdaptiveHuffman {
private:

    struct TreeNode {
        uint32_t symbol;
        uint64_t weight; // Number of times the symbol has been seen
        uint64_t number;

        int parent;
        int left;
        int right;

        bool nyt; // true if not yet transmitted node

        TreeNode() : symbol(0), weight(0), number(0), left(-1), right(-1), parent(-1), nyt(false) {}
    };

    std::vector<TreeNode> tree;
    int nyt_node;
    int next_free_slot;
    int alphabet_size;

    std::vector<int> map; // gets the leaf representing the symbol, indexed by symbol


    inline bool is_leaf(int node) {
        return tree[node].left == -1 && tree[node].right == -1;
    }

    int get_block_leader(int node) {
        // Find the highest number node in nodes of same weight, the nodes are ordered by decreasing number and weight
        int leader = node;
        for (int i = node - 1; i >= 0 && tree[i].weight == tree[leader].weight; i--) {
            if (i != tree[leader].parent) {
                leader = i;
            }
        }

        return leader;
    }

    void swap(int& first, int& second) {
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
            map[tree[first].symbol] = first;
        }
        if (tree[second].symbol != alphabet_size) {
            map[tree[second].symbol] = second;
        }
    }

    void write_symbol(int node, obitstream& out) {
        uint32_t bits; // List of bits that represent the symbol
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
        // Write out the bits in the opposite order, because they need to be from root to leave
        for (int i = 0; i < n; i++) {
            out.writeBit((bits >> i) & 1);
        }
    }

    void slide_and_increment(int node) {
        while (node != -1) {
            int leader = get_block_leader(node);
            if (leader != node) {
                swap(leader, node);
            }

            tree[node].weight++;
            node = tree[node].parent;
        }
    }

    void update_tree(uint32_t symbol) {
        if (map[symbol] == -1) {
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

            map[symbol] = tree[nyt_node].right;
            nyt_node = tree[nyt_node].left;
        }

        slide_and_increment(map[symbol]);
    }

public:
    explicit AdaptiveHuffman(int alphabet_size): alphabet_size(alphabet_size), tree(alphabet_size * 2 + 1), map(alphabet_size, -1), nyt_node(0), next_free_slot(1) {
        tree[0].symbol = alphabet_size;
        tree[0].number = alphabet_size * 2;
        tree[0].nyt = true;
    }

    void encode(uint32_t symbol, obitstream& out) {
        if (map[symbol] != -1) {
            write_symbol(map[symbol], out);
        } else {
            // If the symbol hasn't been seen yet, write the NYT escape sequence
            write_symbol(nyt_node, out);

            // Write the log_2(alphabet_size) + 1 bits fixed representation of the symbol
            for (int i = 8; i >= 0; i--) {
                out.writeBit((symbol >> i) & 1);
            }
        }
        update_tree(symbol);
    }

    int decode(ibitstream& in) {
        int node = 0;

        // Traverse the tree until a leaf is reached
        while (!is_leaf(node)) {
            int bit = in.readBit();
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
            for (int i = 8; i >= 0; i--) {
                number <<= 1;
                int bit = in.readBit();
                if (bit == -1) {
                    return -1;
                }
                number |= bit;
            }
        } else {
            number = tree[node].symbol;;
        }
        if (number >= alphabet_size) {
            return -1;
        }
        update_tree(number);
        return number;
    }
};


#endif //MTF_ADAPTIVEHUFFMAN_H
