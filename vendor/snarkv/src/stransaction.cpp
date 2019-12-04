#include "snark/stransaction.h"
#include "snark/common.h"
#include "snark/librustzcash.h"
#include "snark/signature.h"
#include <fc/log/logger.hpp>

namespace libsnark {
    bool init_zk_params()
    {
        static bool s_inited = false;
        if (s_inited) {
            return true;
        }

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

        s_inited = true;
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
} // end of namespace libsnark


