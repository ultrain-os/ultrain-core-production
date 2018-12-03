#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>

#include <crypto/Digest.h>
#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>
#include <crypto/Signature.h>

#include <stdio.h>
using namespace ultrainio;
using namespace std;

std::string accounts[100] = {
        "genesis",
        "user.111",
        "user.112",
        "user.113",
        "user.114",
        "user.115",
        "user.121",
        "user.122",
        "user.123",
        "user.124",
        "user.125",
        "user.131",
        "user.132",
        "user.133",
        "user.134",
        "user.135",
        "user.141",
        "user.142",
        "user.143",
        "user.144",
        "user.145",
        "user.151",
        "user.152",
        "user.153",
        "user.154",
        "user.155",
        "user.211",
        "user.212",
        "user.213",
        "user.214",
        "user.215",
        "user.221",
        "user.222",
        "user.223",
        "user.224",
        "user.225",
        "user.231",
        "user.232",
        "user.233",
        "user.234",
        "user.235",
        "user.241",
        "user.242",
        "user.243",
        "user.244",
        "user.245",
        "user.251",
        "user.252",
        "user.253",
        "user.254",
        "user.255",
        "user.311",
        "user.312",
        "user.313",
        "user.314",
        "user.315",
        "user.321",
        "user.322",
        "user.323",
        "user.324",
        "user.325",
        "user.331",
        "user.332",
        "user.333",
        "user.334",
        "user.335",
        "user.341",
        "user.342",
        "user.343",
        "user.344",
        "user.345",
        "user.351",
        "user.352",
        "user.353",
        "user.354",
        "user.355",
        "user.411",
        "user.412",
        "user.413",
        "user.414",
        "user.415",
        "user.421",
        "user.422",
        "user.423",
        "user.424",
        "user.425",
        "user.431",
        "user.432",
        "user.433",
        "user.434",
        "user.435",
        "user.441",
        "user.442",
        "user.443",
        "user.444",
        "user.445",
        "user.451",
        "user.452",
        "user.453",
        "user.454"
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "argument error: " << endl;
        cout << "keypair_generator -n 100" << endl;
        return 0;
    }

    int n = atoi(argv[2]);

    std::vector<std::string> skList;
    std::vector<std::string> pkList;
    std::vector<std::string> accountList;
    for (int i = 0; i < n; i++) {
        PrivateKey privateKey;
        PublicKey publicKey;
        PrivateKey::generate(publicKey, privateKey);
        std::string hexPri = std::string(privateKey);
        std::string hexPub = std::string(publicKey);
        cout << "pri:" << hexPri << endl;
        cout << "pub:" << hexPub << endl;
        cout << "account:" << accounts[i] << endl;
        skList.push_back(hexPri);
        pkList.push_back(hexPub);
        accountList.push_back(accounts[i]);
        cout << endl;
    }

    std::cout << "pri key list" << std::endl;
    for (int i = 0; i < n; i++) {
        std::cout << "\"" << skList[i] << "\"," << std::endl;
    }

    std::cout << std::endl << "pub key list" << std::endl;
    for (int i = 0; i < n; i++) {
        std::cout << "\"" << pkList[i] << "\"," << std::endl;
    }

    return 0;
}
