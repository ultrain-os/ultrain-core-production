#pragma once

#include <string>
#include <sstream>
#include "ultrainiolib/block_header.hpp"
#include "BlockHeaderExtKey.h"

namespace {
    block_id_type readBlockId(const std::string& s) {
        block_id_type blk_id;
        std::stringstream ss(s);
        std::string blockIdStr;
        if (!(ss >> blockIdStr)) {
            memcpy(blk_id.hash, blockIdStr.data(), blockIdStr.size());
        }
        return blk_id;
    }
}
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

        static block_id_type getConfirmedBlockId(const ultrainio::block_header& blockHeader) {
            for (auto& e : blockHeader.header_extensions) {
                BlockHeaderExtKey key = static_cast<BlockHeaderExtKey>(std::get<0>(e));
                if (key == kBlsVoterSet) {
                    std::string s;
                    s.assign(std::get<1>(e).begin(), std::get<1>(e).end());
                    return readBlockId(s);
                }
            }
            return block_id_type();
        }
    };
}
