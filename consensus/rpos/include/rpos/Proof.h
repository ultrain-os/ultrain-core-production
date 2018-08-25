#pragma once

#include <crypto/Signature.h>

namespace ultrainio {
    class Proof {
    public:
        explicit Proof(const Signature& signature);

        // hex string
        explicit Proof(const std::string& hexString);

        Proof() = default;

        Proof(const Proof& rhs) = default;

        Proof& operator = (const Proof& rhs) = default;

        explicit operator std::string() const;

        double getRand() const;

        uint32_t getPriority() const;

        bool isValid() const;

        Signature getSignature() const;
    private:
        Signature m_sign;
    };
}