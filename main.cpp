
#include <string>

#include "MTFHash.h"


/*int run_normal_mtf(const std::string& path) {
    std::ifstream ifs(path);
    if (ifs.fail()) {
        return 1;
    }

    std::ostringstream out;
    MTF::encode(ifs, out);
    ifs.close();

    for (auto c: out.str()) {
        std::cout << uint64_t(c) << " ";
    }
    std::cout << std::endl << out.str().size() << std::endl;

    std::istringstream iss_in(out.str());
    std::ostringstream oss;
    RLE::encode(iss_in, oss);

    std::ofstream myfile;
    myfile.open("../out.txt");

    std::cout << oss.str().size() << std::endl;

    compress_final(oss.str().data(), oss.str().size(), myfile);
    myfile.close();

    return 0;
}*/

int main(int argc, char *argv[]) {
    if (argc != 3) {
        return 1;
    }

    std::string path = argv[1];
    int k = (int) strtol(argv[2], nullptr, 10);

    MTFHash::compress(path, "../out.txt", k); // TODO out path in input and default

    return 0;
}
