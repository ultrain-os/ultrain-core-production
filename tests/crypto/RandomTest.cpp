#include <crypto/Random.h>

#include <iostream>

using namespace ultrainio;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "./build/tests/crypto/random_test -t 100" << std::endl;
    }
    int t = atoi(argv[2]);
    std::string pk = std::string("02ba6e9a6e79d202ee47ef7f9c346ad10c6b255946e08d3d9ee8620f1eb53c5db1");
    std::string proof = std::string("03D3155E1819F3C6E573A712FCD3A3FD3DF14A64BCDEF9FEF05241BDAF496FF1125B5F5677892E4EA581DF733B5A6EE9CB29CAD0B63838367C296ECF6E0FFBA3F9787841AFC1383420FDB90CD35C00E2D3");
    std::string msg = std::string("4347873960808359700000000000000000000000000000000000000000000000");
    while (t-- > 0) {
        std::cout << verify_with_pk(const_cast<char*>(pk.data()), const_cast<char*>(proof.data()), const_cast<char*>(msg.data())) << std::endl;
    }
    return 0;
}