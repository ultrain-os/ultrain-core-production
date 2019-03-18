#pragma once

#include "ultrainiolib/block_header.hpp"
#include "BlockHeaderExtKey.h"

namespace ultrainiosystem {
    class ConfirmPoint {
    public:
        static bool isConfirmPoint(const ultrainio::block_header& blockHeader) {
            auto ext = blockHeader.header_extensions;
            for (auto& e : ext) {
                if (std::get<0>(e) == kBlsVoterSet) {
                    return true;
                }
            }
            return false;
        }
    };
}