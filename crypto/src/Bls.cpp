#include <crypto/Bls.h>

#include <cstring>

namespace ultrainio {
    Bls::Bls(char* s, size_t count) {
        if (pairing_init_set_buf(m_pairing, s, count)) pbc_die("pairing init failed");
        element_init_G2(m_g, m_pairing);
    }

    Bls::~Bls() {
        element_clear(m_g);
        pairing_clear(m_pairing);
    }
    bool Bls::initG(unsigned char* g) {
        if (!g) {
            element_random(m_g);
            element_printf("system parameter bytes = %d, g = %B\n", element_length_in_bytes(m_g), m_g);
            return true;
        }
        element_from_bytes(m_g, g);
        return true;
    }

    bool Bls::getG(unsigned char** g, int* gSize) {
        int size = element_length_in_bytes(m_g);
        *g = (unsigned char*)pbc_malloc(size);
        element_to_bytes(*g, m_g);
        *gSize = size;
        return true;
    }

    bool Bls::keygen(unsigned char** sk, int* skSize, unsigned char** pk, int* pkSize) {
        if (!sk || !pk || !skSize || !pkSize) {
            return false;
        }
        element_t skElement;
        element_t pkElement;
        element_init_G2(pkElement, m_pairing);
        element_init_Zr(skElement, m_pairing);
        element_random(skElement);
        int _skSize = element_length_in_bytes(skElement);
        *sk =  (unsigned char*)pbc_malloc(_skSize);
        element_to_bytes(*sk, skElement);
        *skSize = _skSize;
        element_printf("private key = %B\n", skElement);

        // secret key -> public key
        element_pow_zn(pkElement, m_g, skElement);

        int _pkSize = element_length_in_bytes(pkElement);
        *pk = (unsigned char*)pbc_malloc(_pkSize);
        element_to_bytes(*pk, pkElement);
        *pkSize = _pkSize;
        element_printf("public key = %B\n", pkElement);
        element_clear(skElement);
        element_clear(pkElement);
        return true;
    }

    bool Bls::signature(unsigned char* sk, void* hmsg, int hSize, unsigned char** sig, int* sigSize) {
        element_t hElement;
        element_t skElement;
        element_t sigElement;
        element_init_G1(hElement, m_pairing);
        element_init_G1(sigElement, m_pairing);
        element_init_Zr(skElement, m_pairing);

        element_from_bytes(skElement, sk);
        element_from_hash(hElement, hmsg, hSize);
        element_printf("hmsg = %B\n", hElement);
        element_pow_zn(sigElement, hElement, skElement);
        element_printf("signature = %B\n", sigElement);
        *sig = (unsigned char*)pbc_malloc(element_length_in_bytes(sigElement));
        element_to_bytes(*sig, sigElement);

        element_clear(hElement);
        element_clear(skElement);
        element_clear(sigElement);
        return true;
    }

    bool Bls::verify(unsigned char* pk, unsigned char* sig, void* hmsg, int hSize) {
        element_printf("verify\n");
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

        element_from_bytes(sigElement, sig);
        element_printf("signature = %B\n", sigElement);
        element_pairing(temp1, sigElement, m_g);

        element_from_bytes(pkElement, pk);
        element_printf("public key = %B\n", pkElement);
        element_from_hash(hElement, hmsg, hSize);
        element_printf("hmsg = %B\n", hElement);
        element_pairing(temp2, hElement, pkElement);

        bool res = !element_cmp(temp1, temp2);
        element_clear(hElement);
        element_clear(sigElement);
        element_clear(pkElement);
        element_clear(temp1);
        element_clear(temp2);
        return res;
    }

    bool Bls::aggVerify(unsigned char* pk[], unsigned char* sig[], int vec[], int size, void* hmsg, int hSize) {
        element_t pkX;
        element_t sigX;
        element_t hElement;
        element_t temp1;
        element_t temp2;
        element_init_G1(sigX, m_pairing);
        element_init_G1(hElement, m_pairing);
        element_init_G2(pkX, m_pairing);
        element_init_GT(temp1, m_pairing);
        element_init_GT(temp2, m_pairing);
        element_t pkElement;
        element_t sigElement;
        element_init_G2(pkElement, m_pairing);
        element_init_G1(sigElement, m_pairing);
        for (int i = 0; i < size; i++) {
            element_from_bytes(pkElement, pk[i]);
            element_from_bytes(sigElement, sig[i]);
            int q = 1;
            if (vec) {
                q = vec[i];
            }
            for (int j = 0; j < q; j++) {
                element_add(pkX, pkX, pkElement);
                element_add(sigX, sigX, sigElement);
            }
        }
        element_pairing(temp1, sigElement, m_g);
        element_from_hash(hElement, hmsg, hSize);
        element_pairing(temp2, hElement, pkElement);
        bool res = !element_cmp(temp1, temp2);
        element_clear(pkX);
        element_clear(sigX);
        element_clear(hElement);
        element_clear(temp1);
        element_clear(temp2);
        element_clear(pkElement);
        element_clear(sigElement);
        return res;

    }
}