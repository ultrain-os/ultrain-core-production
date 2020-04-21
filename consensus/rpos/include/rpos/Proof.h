#pragma once

#include "core/types.h"

namespace ultrainio {
    class Proof {
    public:
        explicit Proof(const consensus::SignatureType& signature);

        // hex string
        explicit Proof(const std::string& hexString);

        Proof() = default;

        Proof(const Proof& rhs) = default;

        Proof& operator = (const Proof& rhs) = default;

        explicit operator std::string() const;

        double getRand() const;

        uint32_t getPriority() const;

        bool isValid() const;

        consensus::SignatureType getSignature() const;
    private:
        consensus::SignatureType m_sign;
    };
}