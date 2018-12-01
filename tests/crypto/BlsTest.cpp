#include <assert.h>
#include <string.h>

//#include "a.param.h"
#include <crypto/Bls.h>

using ultrainio::Bls;

void readString(char s[], char* fileName);

int main(int argc, char** argv) {
    const char* ultrain = "ultrain";

    char s[16888];
    readString(s, (char*)"/Users/xiaofen/work/source_code/ultrain-core/tests/crypto/a.param");
    //Bls bls(a_param, strlen(a_param));
    Bls bls(s, strlen(s));

    bls.initG(nullptr);

    unsigned char* g;
    int gSize;
    bls.getG(&g, &gSize);
    bls.initG(g);

    unsigned char* sk;
    int skSize;
    unsigned char* pk;
    int pkSize;
    bls.keygen(&sk, &skSize, &pk, &pkSize);

    unsigned char* sig;
    int sigSize;
    bls.signature(sk, (void*)ultrain, 7, &sig, &sigSize);
    assert(bls.verify(pk, sig, (void*)ultrain, 7) == true);

    unsigned char* randomSig;
    int randomSigSize;
    bls.signature(sk, (void*)"ultraiN", 7, &randomSig, &randomSigSize);
    assert(bls.verify(pk, randomSig, (void*)ultrain, 7) == false);

    unsigned char* sk1;
    int sk1Size;
    unsigned char* pk1;
    int pk1Size;
    bls.keygen(&sk1, &sk1Size, &pk1, &pk1Size);

    unsigned char* sig1;
    int sig1Size;
    bls.signature(sk1, (void*)ultrain, 7, &sig1, &sig1Size);
    assert(bls.verify(pk1, sig1, (void*)ultrain, 7) == true);

    unsigned char* sk2;
    int sk2Size;
    unsigned char* pk2;
    int pk2Size;
    bls.keygen(&sk2, &sk2Size, &pk2, &pk2Size);

    unsigned char* sig2;
    int sig2Size;
    bls.signature(sk2, (void*)ultrain, 7, &sig2, &sig2Size);
    assert(bls.verify(pk2, sig2, (void*)ultrain, 7) == true);

    element_printf("aggverify\n");
    unsigned char* pks[3];
    unsigned char* sigs[3];
    pks[0] = pk;
    pks[1] = pk1;
    pks[2] = pk2;
    sigs[0] = sig;
    sigs[1] = sig1;
    sigs[2] = sig2;
    int vec[3];
    vec[0] = 1;
    vec[1] = 2;
    vec[2] = 3;
    assert(bls.aggVerify(pks, sigs, vec, 3, (void*)ultrain, 7) == true);

    pbc_free(g);
    pbc_free(sk);
    pbc_free(pk);
    pbc_free(sig);
    pbc_free(sk1);
    pbc_free(pk1);
    pbc_free(sig1);
    pbc_free(sk2);
    pbc_free(pk2);
    pbc_free(sig2);
    pbc_free(randomSig);
    return 0;
}

void readString(char s[], char* fileName) {
    FILE *fp = stdin;

    fp = fopen(fileName, "r");
    if (!fp) pbc_die("error opening %s", fileName);
    size_t count = fread(s, 1, 16384, fp);
    if (!count) pbc_die("input error");
    fclose(fp);
}