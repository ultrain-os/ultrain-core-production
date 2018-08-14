#pragma once

#include <pbc.h>

namespace ultrainio {
    class Bls {
    public:
        Bls(char* s, size_t count);
        ~Bls();
        bool initG(unsigned char* g);
        bool getG(unsigned char** g, int* gSize);
        bool keygen(unsigned char** sk, int* skSize, unsigned char** pk, int* pkSize);
        bool signature(unsigned char* sk, void* hmsg, int hSize, unsigned char** sig, int* sigSize);
        bool verify(unsigned char* pk, unsigned char* sig, void* hmsg, int hSize);
        bool aggVerify(unsigned char* pk[], unsigned char* sig[], int vec[], int size, void* hmsg, int hSize);

    private:
        pairing_t m_pairing;
        element_t m_g;
    };
}