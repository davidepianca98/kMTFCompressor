
#include <string>
#include <fstream>
#include <sstream>

#include "MTFHash.h"
#include "MTF.h"
#include "Core.h"


/*int run_normal_mtf(const std::string& path) {
    std::ifstream in_file(path);
    if (in_file.fail()) {
        return 1;
    }

    std::ostringstream out;
    MTF::encode(in_file, out);
    in_file.close();

    std::ofstream myfile;
    myfile.open("../out.txt");

    Core::compress_final(out.str().data(), out.str().size(), myfile);
    myfile.close();

    return 0;
}*/

int main(int argc, char *argv[]) {
    if (argc != 3) {
        return 1;
    }

    std::string path = argv[1];
    int k = (int) strtol(argv[2], nullptr, 10);

    MTFHash::compress_stream(path, "../out.txt", k); // TODO out path in input and default

    return 0;
}
