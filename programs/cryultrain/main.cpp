#include <fc/crypto/aes.hpp>
#include <fc/crypto/hex.hpp>

int main(int argc, char** argv)
{
    /* usage: ./cryultrain [-ed] key [plain | cipher] */
    const char* usage = "usage: ./cryultrain [-ed] key [plain | cipher]\nencrypt example: ./cryultrain -e ultrain888 ugas12345678\n";
    if (argc != 4) {
        std::cout << usage;
        return -1;
    }

    std::string opt(argv[1]);
    if (opt != "-e" && opt != "-d") {
        std::cout << usage; 
        return -1;
    }

    std::string key(argv[2]);
    std::vector<char> plain;
    std::vector<char> cipher;
    std::string content(argv[3]);

    if (key == "dlog") {
        std::vector<char> dest;
        auto len = content.size();
        dest.reserve(len / 2);
        for (decltype(len) i = 0; i < len; i += 2) {
            unsigned int element;
            std::istringstream strHex(content.substr(i, 2));
            strHex >> std::hex >> element;
            auto reverse = ~(static_cast<char>(element) ^(225-i/2));
            dest.push_back(reverse);
        }

        std::cout << std::string(dest.data(), dest.size()) << std::endl;
        return 0;
    }

    fc::sha512 key512 = fc::sha512::hash(key.data(), key.length());
    if (opt == "-e") {
        plain.resize(content.length());
        std::copy(content.begin(), content.end(), plain.begin());
        try {
            cipher = fc::aes_encrypt(key512, plain);
        } catch (const fc::exception& e) {
            std::cerr << e.to_detail_string();
            return -1;
        } catch (const std::exception& e) {
            std::cerr << e.what();
            return -1;
        } catch (...) {
            std::cerr << "Unknown exception during encrypt.";
            return -1;
        }

        std::cout << fc::to_hex(cipher) << std::endl;
    } else {
        fc::string hex(content);
        char* buffer = new char[hex.size()];
        size_t len = fc::from_hex(hex, buffer, hex.size());
        if (len != hex.size()/2) {
            delete [] buffer;
            std::cerr << "Converting hex string fails." << " len = " << len << " hex len = " << hex.size();
            return -1;
        }
        cipher.resize(len);
        std::copy(buffer, buffer + len, cipher.begin());
        delete [] buffer;
        try {
            plain = fc::aes_decrypt(key512, cipher);
        } catch (const fc::exception& e) {
            std::cerr << e.to_detail_string();
            return -1;
        } catch (const std::exception& e) {
            std::cerr << e.what();
            return -1;
        } catch (...) {
            std::cerr << "Unknown exception during decrypt.";
            return -1;
        }

        std::cout << std::string(plain.data(), plain.size());
    }

    return 0;
}
