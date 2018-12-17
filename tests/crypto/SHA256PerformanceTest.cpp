#include <fc/crypto/sha256.hpp>

#include <chrono>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "argument count < 3";
        return 0;
    }
    int n = atoi(argv[2]);
    std::string* m = new std::string[n];
    std::chrono::steady_clock::time_point pointStart = std::chrono::steady_clock::now();
    for (int i = 0; i < n; i++) {
        fc::sha256::hash(m[i]);
    }
    std::chrono::microseconds d = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - pointStart);
    std::cout << "hash each one consume : " << d.count() / n << " microseconds " << std::endl;
    delete []m;
    return 0;
}