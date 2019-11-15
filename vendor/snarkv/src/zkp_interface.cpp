#include <ctime>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <memory>
#include <regex>
#include <exception>  


// #include <boost/optional/optional_io.hpp>
#include "common/utils.hpp"
#include "algebra/fields/field_utils.hpp"
#include "algebra/curves/public_params.hpp"

#include "common/default_types/r1cs_ppzksnark_pp.hpp"
#include "zk_proof_systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark.hpp"
#include "zk_proof_systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark_params.hpp"

#include "common/default_types/r1cs_gg_ppzksnark_pp.hpp"
#include "zk_proof_systems/ppzksnark/r1cs_gg_ppzksnark/r1cs_gg_ppzksnark.hpp"
#include "zk_proof_systems/ppzksnark/r1cs_gg_ppzksnark/r1cs_gg_ppzksnark_params.hpp"


#include "common/data_structures/accumulation_vector.hpp"
#include "algebra/knowledge_commitment/knowledge_commitment.hpp"
#include "snark/zkp_interface.h"
#include "snark/librustzcash.h"
#include "snark/signature.h"
#include "snark/common.h"
#include "snark/util.h"
#include <boost/filesystem.hpp>
#include <fc/log/logger.hpp>

using namespace libsnark;
using namespace std;

namespace  libsnark {
    template<typename var>
    var load_val(char* str)
    {
        var val;
        std::istringstream ss(str);
        ss >> val;
        return val;
    }

    void init_pp_public_params()
    {
        alt_bn128_pp::init_public_params();
    }
    
    //Groth16
    bool verify_zero_knowledge_proof(char *vkStr, char *pm_inputStr, char *proofStr)
    {
        if (!vkStr || !pm_inputStr || !proofStr) {
            elog("vkStr or pm_inputStr or proofStr is NULL");
            return false;
        }

        try
        {
            typedef alt_bn128_pp ppT;
            ppT::init_public_params();

            auto vk = load_val<r1cs_gg_ppzksnark_verification_key<ppT>>(vkStr);
            /*size_t size = 0;
            size = load_val<size_t>(pm_inputStr);
            if (size == 0 || size != vk.encoded_IC_query.domain_size())
            {
                elog("primary input size ${s} error or vk domain_size ${ds} error.", ("s", size)("ds", vk.encoded_IC_query.domain_size()));
                return false;
            }*/
	    
            auto primary_input = load_val<r1cs_primary_input<Fr<ppT>>>(pm_inputStr);
            auto proof = load_val<r1cs_gg_ppzksnark_proof<ppT>>(proofStr);

            if (!proof.is_well_formed())
            {
                elog("incorrect proof!");
                return false;
            }

            if (vk.encoded_IC_query.domain_size() < primary_input.size())
            {
                elog("mismatched vk size and primary input size!");
                return false;
            }

            const accumulation_vector<G1<ppT>> accumulated_IC =
                vk.encoded_IC_query.template accumulate_chunk<Fr<ppT>>(primary_input.begin(), primary_input.end(), 0);

            if (!(accumulated_IC.is_fully_accumulated()))
            {
                elog("accumulated IC in vk is not fully accumulated!");
                return false;
            }

            if (!accumulated_IC.first.is_well_formed())
            {
                elog("incorrect G1 point in Accumulator!");
                return false;
            }

            if (!accumulated_IC.rest.is_valid())
            {
                elog("incorrect sparse_vector!");
                return false;
            }

            const G1<ppT> &acc = accumulated_IC.first;
            auto G1_one = G1<ppT>::one();
            auto G2_one = G2<ppT>::one();

            if (!proof.is_well_formed())
            {
                elog("proof.is_well_formed()");
                return false;
            }

            const G1_precomp<ppT> proof_g_A_precomp = ppT::precompute_G1(proof.g_A);
            const G2_precomp<ppT> proof_g_B_precomp = ppT::precompute_G2(proof.g_B);
            const G1_precomp<ppT> proof_g_C_precomp = ppT::precompute_G1(proof.g_C);
            const G1_precomp<ppT> acc_precomp = ppT::precompute_G1(acc);

            auto pvk_vk_gamma_g2_precomp = ppT::precompute_G2(vk.gamma_g2);
            auto pvk_vk_delta_g2_precomp = ppT::precompute_G2(vk.delta_g2);

            const Fqk<ppT> QAP1 = ppT::miller_loop(proof_g_A_precomp, proof_g_B_precomp);
            const Fqk<ppT> QAP2 = ppT::double_miller_loop(
                acc_precomp, pvk_vk_gamma_g2_precomp,
                proof_g_C_precomp, pvk_vk_delta_g2_precomp);
            const GT<ppT> QAP = ppT::final_exponentiation(QAP1 * QAP2.unitary_inverse());

            if (QAP != vk.alpha_g1_beta_g2)
                return false;

            return true;
        } catch (const std::exception& e) {
            elog("verify zkp failure, std exception: ${e}", ("e", e.what()));
            return false;
        } catch (...) {
            elog("unknown exeption, verify zkp failure.");
            return false;
        }
    }

    // BCTV14
    template <typename ppT>
    bool f_sub1(G1<ppT> g1_1, G2<ppT> g2_1, G1<ppT> g1_2, G2<ppT> g2_2, G1<ppT> g1_3, G2<ppT> g2_3, int option)
    {
        G1_precomp<ppT> preG1_1 = ppT::precompute_G1(g1_1);
        G2_precomp<ppT> preG2_1 = ppT::precompute_G2(g2_1);
        G1_precomp<ppT> preG1_2 = ppT::precompute_G1(g1_2);
        G2_precomp<ppT> preG2_2 = ppT::precompute_G2(g2_2);

        Fqk<ppT> kc1 = ppT::miller_loop(preG1_1, preG2_1);
        Fqk<ppT> kc2;
        Fqk<ppT> kc_A_2;

        if(option==1)
            kc2 = ppT::miller_loop(preG1_2, preG2_2);
        else
        {
            G1_precomp<ppT> preG1_3 = ppT::precompute_G1(g1_3);
            G2_precomp<ppT> preG2_3 = ppT::precompute_G2(g2_3);
            kc2 = ppT::double_miller_loop(preG1_2, preG2_2, preG1_3, preG2_3);
        }

        GT<ppT> kc_A = ppT::final_exponentiation(kc1 * kc2.unitary_inverse());
        if (kc_A != GT<ppT>::one())
            return false;
        
        return true;
    }
    
    bool verify_zero_knowledge_proof_BCTV14(char *vkStr, char *pm_inputStr, char *proofStr)
    {
        if (!vkStr || !pm_inputStr || !proofStr) {
            elog("vkStr or pm_inputStr or proofStr is NULL");
            return false;
        }

        try
        {
            default_r1cs_ppzksnark_pp::init_public_params();
            typedef default_r1cs_ppzksnark_pp ppT;
            typedef Fr<ppT> FieldT;

            try {
                // if(!check_pattern(verify_key, primary_input, proof)) 
                //     return false;
            } catch (...) {
                elog("check pattern exception");
                return false;
            }

            auto vk = load_val<r1cs_ppzksnark_verification_key<ppT>>(vkStr);
            auto primary_input = load_val<r1cs_primary_input<FieldT>>(pm_inputStr);
            auto proof = load_val<r1cs_ppzksnark_proof<ppT>>(proofStr);

            if (!proof.is_well_formed())
            {
                elog("incorrect proof!");
                return false;
            }

            if (vk.encoded_IC_query.domain_size() < primary_input.size())
            {
                elog("mismatched vk size and primary input size!");
                return false;
            }

            const accumulation_vector<G1<ppT>> accumulated_IC =
                vk.encoded_IC_query.template accumulate_chunk<Fr<ppT>>(primary_input.begin(), primary_input.end(), 0);

            if (!(accumulated_IC.is_fully_accumulated()))
            {
                elog("accumulated IC in vk is not fully accumulated!");
                return false;
            }

            if (!accumulated_IC.first.is_well_formed())
            {
                elog("incorrect G1 point in Accumulator!");
                return false;
            }

            if (!accumulated_IC.rest.is_valid())
            {
                elog("incorrect sparse_vector!");
                return false;
            }

            const G1<ppT> &acc = accumulated_IC.first;
            auto G1_one = G1<ppT>::one();
            auto G2_one = G2<ppT>::one();

            if (!proof.is_well_formed())
            {
                elog("proof is not well formed");
                return false;
            }

            if (!(f_sub1<ppT>(proof.g_A.g, vk.alphaA_g2, proof.g_A.h, G2_one, G1_one, G2_one, 1)))
            {
                elog("f_sub1<ppT>(proof.g_A.g) failure ");
                return false;
            }

            if (!(f_sub1<ppT>(vk.alphaB_g1, proof.g_B.g, proof.g_B.h, G2_one, G1_one, G2_one, 1)))
            {
                elog("vk.alphaB_g1 failure");
                return false;
            }

            if (!(f_sub1<ppT>(proof.g_C.g, vk.alphaC_g2, proof.g_C.h, G2_one, G1_one, G2_one, 1)))
            {
                elog("f_sub1<ppT>(proof.g_C.g) failure");
                return false;
            }

            if (!(f_sub1<ppT>(proof.g_A.g + acc, proof.g_B.g, proof.g_H, vk.rC_Z_g2, proof.g_C.g, G2_one, 2)))
            {
                elog("f_sub1<ppT>(proof.g_A.g + acc) failure");
                return false;
            }

            if (!(f_sub1<ppT>(proof.g_K, vk.gamma_g2, (proof.g_A.g + acc) + proof.g_C.g, vk.gamma_beta_g2, vk.gamma_beta_g1, proof.g_B.g, 2)))
            {
                elog("f_sub1<ppT>(proof.g_K, vk) failure");
                return false;
            }

            return true;
        } catch (const std::exception& e) {
            elog("verify zkp BCTV14 failure. ${e}", ("e", e.what()));
            return false;
        } catch(...) {
            elog("unknown exeption, verify zkp BCTV14 failure.");
            return false;
        }
    }

    bool interface_verify(char *chr)
    {
        if (!chr) {
            elog("input param is NULL");
            return false;
        }

        char *vkStr = strtok(chr, ",");
        char *pmInputStr = strtok(NULL, ",");
        char *proofStr = strtok(NULL, ",");

	if (!vkStr || !pmInputStr || !proofStr) {
            elog("parse error!");
            return false;
        }

        auto res = verify_zero_knowledge_proof(vkStr, pmInputStr, proofStr);

        if (res) {
            ilog("verification OK!");
        } else {
            elog("verification failed!");
        }

        return res;
    }

    /// Sprout JoinSplit proof verification.
    bool sprout_verify(
        const unsigned char *proof,
        const unsigned char *rt,
        const unsigned char *h_sig,
        const unsigned char *mac1,
        const unsigned char *mac2,
        const unsigned char *nf1,
        const unsigned char *nf2,
        const unsigned char *cm1,
        const unsigned char *cm2,
        uint64_t vpub_old,
        uint64_t vpub_new)
    {
        return librustzcash_sprout_verify(proof, rt, h_sig, mac1, mac2, nf1, nf2, cm1, cm2, vpub_old, vpub_new);
    }

    bool init_zk_params()
    {
        if (init_and_check_sodium() == -1) {
            elog("init_and_check_sodium failed");
            return false;
        }

        ilog("*****************init params success************");
        boost::filesystem::path sapling_spend = ZC_GetParamsDir() / "sapling-spend.params";
        boost::filesystem::path sapling_output = ZC_GetParamsDir() / "sapling-output.params";
        boost::filesystem::path sprout_groth16 = ZC_GetParamsDir() / "sprout-groth16.params";

        static_assert(
            sizeof(boost::filesystem::path::value_type) == sizeof(codeunit),
            "librustzcash not configured correctly");
        auto sapling_spend_str = sapling_spend.native();
        auto sapling_output_str = sapling_output.native();
        auto sprout_groth16_str = sprout_groth16.native();

        ilog("sapling spend: ${ss}", ("ss", sapling_spend_str.c_str()));
        ilog("sapling output: ${so}", ("so", sapling_output_str.c_str()));
        ilog("sprout groth16: ${sg}", ("sg", sprout_groth16_str.c_str()));

        librustzcash_init_zksnark_params(
            reinterpret_cast<const codeunit*>(sapling_spend_str.c_str()),
            sapling_spend_str.length(),
            "8270785a1a0d0bc77196f000ee6d221c9c9894f55307bd9357c3f0105d31ca63991ab91324160d8f53e2bbd3c2633a6eb8bdf5205d822e7f3f73edac51b2b70c",
            reinterpret_cast<const codeunit*>(sapling_output_str.c_str()),
            sapling_output_str.length(),
            "657e3d38dbb5cb5e7dd2970e8b03d69b4787dd907285b5a7f0790dcc8072f60bf593b32cc2d1c030e00ff5ae64bf84c5c3beb84ddc841d48264b4a171744d028",
            reinterpret_cast<const codeunit*>(sprout_groth16_str.c_str()),
            sprout_groth16_str.length(),
            "e9b238411bd6c0ec4791e9d04245ec350c9c5744f5610dfcce4365d5ca49dfefd5054e371842b3f88fa1b9d7e8e075249b3ebabd167fa8b0f3161292d36c180a"
        );

        return true;
    }

    bool check_shielded_transaction(const shielded_transaction& tx)
    {
        const std::vector<SpendDescription>& spend_vec = tx.vShieldedSpend;
        const std::vector<OutputDescription>& output_vec = tx.vShieldedOutput;
        zero::uint256 signed_hash;

        if (!spend_vec.empty() || !output_vec.empty())
        {
            try {
                signed_hash = SignatureHash(tx, NOT_AN_INPUT, SIGHASH_ALL, 0);
            } catch (std::logic_error ex) {
                elog("Signature hash for shielded transaction failed!");
                return false;
            }

            auto ctx = librustzcash_sapling_verification_ctx_init();

            for (const SpendDescription& spend : spend_vec) {
                if (!librustzcash_sapling_check_spend(
                    ctx,
                    spend.cv.begin(),
                    spend.anchor.begin(),
                    spend.nullifier.begin(),
                    spend.rk.begin(),
                    spend.zkproof.begin(),
                    spend.spendAuthSig.begin(),
                    signed_hash.begin()))
                {
                    librustzcash_sapling_verification_ctx_free(ctx);
	            elog("spend check failure");
                    return false;
                }
            }

            for (const OutputDescription& output : output_vec) {
                if (!librustzcash_sapling_check_output(
                    ctx,
                    output.cv.begin(),
                    output.cm.begin(),
                    output.ephemeralKey.begin(),
                    output.zkproof.begin()))
                {
                    librustzcash_sapling_verification_ctx_free(ctx);
                    elog("output check failure");
                    return false;
                }
            }

            if (!librustzcash_sapling_final_check(
                ctx,
                tx.valueBalance,
                tx.bindingSig.begin(),
                signed_hash.begin()))
            {
                librustzcash_sapling_verification_ctx_free(ctx);
                elog("final check failure");
                return false;
            }

            librustzcash_sapling_verification_ctx_free(ctx);
        }
        return true;
    }
}
