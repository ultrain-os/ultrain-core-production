#include <crypto/Bls.h>

#include <cstring>
#include <base/Hex.h>

#include <fc/crypto/sha256.hpp>
#include <fc/exception/exception.hpp>

namespace ultrainio {
    const int Bls::BLS_PRI_KEY_LENGTH = 20;

    const int Bls::BLS_PUB_KEY_COMPRESSED_LENGTH = 65;

    const int Bls::BLS_SIGNATURE_COMPRESSED_LENGTH = 65;

    const int Bls::BLS_G_LENGTH = 128;

    const std::string Bls::ULTRAINIO_BLS_DEFAULT_PARAM = std::string("type a q 8780710799663312522437781984754049815806883199414208211028653399266475630880222957078625179422662221423155858769582317459277713367317481324925129998224791 h 12016012264891146079388821366740534204802954401251311822919615131047207289359704531102844802183906537786776 r 730750818665451621361119245571504901405976559617 exp2 159 exp1 107 sign1 1 sign0 1");

    const std::string Bls::ULTRAINIO_BLS_DEFAULT_G = std::string("a08f52063b32a90a1566af1bf797d328747dc6ec4c5ff8739577dffcdd8552537664d40cf0318984509b2cc6bd6d9b8dedae707a02df7ef1eae5ecd0114454bd2b9e8a42982753cf78604c62a68653ac855b3a6673f1ff6fe3399d3c0705b951faf714574896a652ea2990448cdec114eff0f738b7f90795d8df734cce865ac2");

    std::shared_ptr<Bls> Bls::s_blsPtr = nullptr;
    Bls::Bls(const char* s, size_t count) {
        if (pairing_init_set_buf(m_pairing, s, count)) pbc_die("pairing init failed");
        element_init_G2(m_g, m_pairing);
    }

    Bls::~Bls() {
        element_clear(m_g);
        pairing_clear(m_pairing);
    }

    bool Bls::getSk(const std::string& boringsslSk, unsigned char* sk, int skSize) {
        if (skSize < Bls::BLS_PRI_KEY_LENGTH) {
            elog("sk size less Bls::BLS_PRI_KEY_LENGTH");
            return false;
        }
        std::string hash = fc::sha256::hash(boringsslSk).str();
        std::string skHex = hash.substr(0, Bls::BLS_PRI_KEY_LENGTH * 2);
        return Hex::fromHex<unsigned char>(skHex, sk, skSize) == Bls::BLS_PRI_KEY_LENGTH;
    }

    std::shared_ptr<Bls> Bls::getDefault() {
        if (!s_blsPtr) {
            s_blsPtr = std::make_shared<Bls>(Bls::ULTRAINIO_BLS_DEFAULT_PARAM.c_str(), strlen(Bls::ULTRAINIO_BLS_DEFAULT_PARAM.c_str()));
            unsigned char defaultG[Bls::BLS_G_LENGTH];
            Hex::fromHex<unsigned char>(ULTRAINIO_BLS_DEFAULT_G, defaultG, Bls::BLS_G_LENGTH);
            s_blsPtr->initG(defaultG);
        }
        return s_blsPtr;
    }

    bool Bls::initG(unsigned char* g) {
        if (!g) {
            element_random(m_g);
            return true;
        }
        element_from_bytes(m_g, g);
        return true;
    }

    bool Bls::getG(unsigned char* g, int gSize) {
        if (gSize < element_length_in_bytes(m_g)) {
            return false;
        }
        element_to_bytes(g, m_g);
        return true;
    }

    bool Bls::keygen(unsigned char* sk, int skSize, unsigned char* pk, int pkSize) {
        if (sk == nullptr || skSize < Bls::BLS_PRI_KEY_LENGTH || pk == nullptr
                || pkSize < Bls::BLS_PUB_KEY_COMPRESSED_LENGTH) {
            return false;
        }
        element_t skElement;
        element_t pkElement;
        element_init_G2(pkElement, m_pairing);
        element_init_Zr(skElement, m_pairing);
        element_random(skElement);
        element_to_bytes(sk, skElement);
        // secret key -> public key
        element_pow_zn(pkElement, m_g, skElement);
        element_to_bytes_compressed(pk, pkElement);
        element_clear(skElement);
        element_clear(pkElement);
        return true;
    }

    bool Bls::getPk(unsigned char* pk,int pkSize, unsigned char* sk, int skSize) {
        if (pk == nullptr || pkSize < Bls::BLS_PUB_KEY_COMPRESSED_LENGTH || sk == nullptr
                || skSize < Bls::BLS_PRI_KEY_LENGTH) {
            return false;
        }
        element_t skElement;
        element_t pkElement;
        element_init_G2(pkElement, m_pairing);
        element_init_Zr(skElement, m_pairing);
        element_from_bytes(skElement, sk);
        element_pow_zn(pkElement, m_g, skElement);
        ilog("pk compress length: ${compress_length}, normal length: ${length}", ("compress_length", element_length_in_bytes_compressed(pkElement))("length", element_length_in_bytes(pkElement)));
        element_to_bytes_compressed(pk, pkElement);
        element_clear(skElement);
        element_clear(pkElement);
        return true;
    }

    bool Bls::sign(unsigned char* sk, void* hmsg, int hSize, unsigned char* sig, int sigSize) {
        if (!sk || !hmsg || !sig || sigSize < Bls::BLS_SIGNATURE_COMPRESSED_LENGTH) {
            return false;
        }
        element_t hElement;
        element_t skElement;
        element_t sigElement;
        element_init_G1(hElement, m_pairing);
        element_init_G1(sigElement, m_pairing);
        element_init_Zr(skElement, m_pairing);

        element_from_bytes(skElement, sk);
        element_from_hash(hElement, hmsg, hSize);
        element_pow_zn(sigElement, hElement, skElement);
        //ilog("sig compress length: ${compress_length}, normal length: ${length}", ("compress_length", element_length_in_bytes_compressed(sigElement))("length", element_length_in_bytes(sigElement)));
        element_to_bytes_compressed(sig, sigElement);
        element_clear(hElement);
        element_clear(skElement);
        element_clear(sigElement);
        return true;
    }

    bool Bls::verify(unsigned char* pk, unsigned char* sig, void* hmsg, int hSize) {
        if (!pk || !sig || !hmsg) {
            return false;
        }
        element_t sigElement;
        element_t hElement;
        element_t pkElement;
        element_t temp1;
        element_t temp2;
        element_init_G1(hElement, m_pairing);
        element_init_G1(sigElement, m_pairing);
        element_init_G2(pkElement, m_pairing);
        element_init_GT(temp1, m_pairing);
        element_init_GT(temp2, m_pairing);

        element_from_bytes_compressed(sigElement, sig);
        element_pairing(temp1, sigElement, m_g);

        element_from_bytes_compressed(pkElement, pk);
        element_from_hash(hElement, hmsg, hSize);
        element_pairing(temp2, hElement, pkElement);

        bool res = !element_cmp(temp1, temp2);
        element_clear(hElement);
        element_clear(sigElement);
        element_clear(pkElement);
        element_clear(temp1);
        element_clear(temp2);
        return res;
    }

    bool Bls::verifyKeyPair(unsigned char* sk, int skSize, unsigned char* pk, int pkSize) {
        unsigned char signature[Bls::BLS_SIGNATURE_COMPRESSED_LENGTH];
        std::string h("ultrain");
        bool res = sign(sk, (void*)h.c_str(), h.length(), signature, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
        if (!res) {
            return false;
        }
        return verify(pk, signature, (void*)h.c_str(), h.length());
    }

    bool Bls::aggregate(unsigned char* sig[], int size, unsigned char* sigX, int sigXSize) {
        if (!sig || sigXSize < Bls::BLS_SIGNATURE_COMPRESSED_LENGTH) {
            return false;
        }
        element_t sigXElement;
        element_init_G1(sigXElement, m_pairing);
        element_t sigElement;
        element_init_G1(sigElement, m_pairing);
        for (int i = 0; i < size; i++) {
            if (!sig[i]) {
                elog("sig vector has nullptr");
                element_clear(sigXElement);
                element_clear(sigElement);
                return false;
            }
            element_from_bytes_compressed(sigElement, sig[i]);
            element_add(sigXElement, sigXElement, sigElement);
        }
        element_to_bytes_compressed(sigX, sigXElement);
        //ilog("sigX compressed length : ${compressed_length}, normal length :${length}", ("compressed_length", element_length_in_bytes_compressed(sigXElement))("length", element_length_in_bytes(sigXElement)));
        element_clear(sigXElement);
        element_clear(sigElement);
        return true;
    }

    bool Bls::verifyAggregate(unsigned char* pk[], int size, unsigned char* sigX, int sigXSize, void* hmsg, int hSize) {
        if ((!pk) || (!sigX) || (sigXSize < Bls::BLS_SIGNATURE_COMPRESSED_LENGTH) || (!hmsg)) {
            return false;
        }
        element_t pkXElement;
        element_t hElement;
        element_t temp1;
        element_t temp2;
        element_t sigXElement;
        element_init_G1(sigXElement, m_pairing);
        element_from_bytes_compressed(sigXElement, sigX);
        element_init_G1(hElement, m_pairing);
        element_init_G2(pkXElement, m_pairing);
        element_init_GT(temp1, m_pairing);
        element_init_GT(temp2, m_pairing);
        element_t pkElement;
        element_init_G2(pkElement, m_pairing);
        for (int i = 0; i < size; i++) {
            if (!pk[i]) {
                elog("pk vector has nullptr");
                element_clear(pkXElement);
                element_clear(hElement);
                element_clear(temp1);
                element_clear(temp2);
                element_clear(sigXElement);
                element_clear(pkElement);
                return false;
            }
            element_from_bytes_compressed(pkElement, pk[i]);
            element_add(pkXElement, pkXElement, pkElement);
        }
        element_pairing(temp1, sigXElement, m_g);
        element_from_hash(hElement, hmsg, hSize);
        element_pairing(temp2, hElement, pkXElement);
        bool res = !element_cmp(temp1, temp2);
        element_clear(pkXElement);
        element_clear(hElement);
        element_clear(temp1);
        element_clear(temp2);
        element_clear(sigXElement);
        element_clear(pkElement);
        return res;
    }
}
