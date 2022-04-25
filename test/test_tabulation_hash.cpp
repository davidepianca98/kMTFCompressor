
#include "hash/randomized/TabulationHash.h"

int main() {
    TabulationHash hash(5, 1);

    hash.update('a');
    hash.update('b');
    hash.update('c');
    hash.update('d');
    hash.update('e');

    uint64_t h1 = hash.get_hash();

    hash.update('a');
    hash.update('b');
    hash.update('c');
    hash.update('d');
    hash.update('e');

    uint64_t h2 = hash.get_hash();

    if (h1 != h2) {
        return 1;
    }

    uint64_t kmer = 0;
    kmer = (kmer << 8) | 'a';
    kmer = (kmer << 8) | 'b';
    kmer = (kmer << 8) | 'c';
    kmer = (kmer << 8) | 'd';
    kmer = (kmer << 8) | 'e';

    uint64_t h3 = hash.compute(kmer);

    if (h2 != h3) {
        return 1;
    }

    return 0;
}
