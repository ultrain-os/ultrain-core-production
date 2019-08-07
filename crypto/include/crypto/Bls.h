#pragma once

#include <memory>
#include <string>

#include <pbc.h>

namespace ultrainio {
    class Bls {
    public:
        static const int BLS_PRI_KEY_LENGTH;
        static const int BLS_PUB_KEY_COMPRESSED_LENGTH;
        static const int BLS_SIGNATURE_COMPRESSED_LENGTH;
        static const int BLS_G_LENGTH;
        static const std::string ULTRAINIO_BLS_DEFAULT_PARAM;
        static const std::string ULTRAINIO_BLS_DEFAULT_G;

        Bls(const char* s, size_t count);
        ~Bls();
        static std::shared_ptr<Bls> getDefault();
        bool initG(unsigned char* g);
        bool getG(unsigned char* g, int gSize);
        bool keygen(unsigned char* sk, int skSize, unsigned char* pk, int pkSize);
        bool getPk(unsigned char* pk,int pkSize, unsigned char* sk, int skSize);
        bool verifyKeyPair(unsigned char* sk, int skSize, unsigned char* pk, int pkSize);
        bool sign(unsigned char* sk, void* hmsg, int hSize, unsigned char* sig, int sigSize);
        bool verify(unsigned char* pk, unsigned char* sig, void* hmsg, int hSize);
        bool aggregate(unsigned char* sig[], int size, unsigned char* sigX, int sigXSize);
        bool verifyAggregate(unsigned char* pk[], int size, unsigned char* sigX, int sigXSize, void* hmsg, int hSize);

    private:
        static std::shared_ptr<Bls> s_blsPtr;
        pairing_t m_pairing;
        element_t m_g;
    };
}