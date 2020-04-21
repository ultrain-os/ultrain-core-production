#pragma once

#include <string>

namespace ed25519 {
    class Digest {
    public:
        explicit Digest(const std::string& h);
        explicit operator std::string() const;

    private:
        std::string m_h;
    };
}