
#include <bitset>
#include "encoders/AdaptiveHuffman.h"
#include "stream/obitstream/obufbitstream.h"

int main() {
    AdaptiveHuffman ah(256);
    uint8_t buffer[256];
    obufbitstream out(buffer, 256);

    std::string s = "mississippi";
    //std::string s = "aa bbb cccc ddddd eeeeee fffffffgggggggg";
    //std::string s = "e eae de eabe eae dcf";
    //std::string s = "aa bbb c";
    //std::string s = "abb";
    //std::string s = "abcdefghijklmnopqrstuvwxyz";
    //std::string s = "abbcccdddddeeeeeeeefffffffffffffggggggggggggggggggggg";
    //std::string s = "abbcccdddddeeeeeeeefffffffffffffggggggggggggggggggggghhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii";
    for (int i = 0; i < s.length(); i++) {
        //std::cout << s[i] << " ";
        ah.print_tree();
        std::cout << std::endl;
        ah.encode(s[i], out);
    }
    ah.print_tree();
    std::cout << std::endl;

    out.flush_remaining();

    for (int i = 0; i < out.size(); i++) {
        std::cout << std::bitset<8>(buffer[i]);
    }

    return 0;
}
