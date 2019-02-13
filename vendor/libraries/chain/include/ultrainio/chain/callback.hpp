#pragma once

#include <stdint.h>
#include <string>

namespace ultrainio { namespace chain {
    // interface
    class callback {
    public:
        virtual ~callback();
        virtual int on_header_extensions_verify(uint64_t chain_name, int ext_key, const std::string& ext_value) = 0;
    };
} } // namespace ultrainio::chain