#pragma once

#include <string>
#include "ultrainiolib/block_header.hpp"
#include "BlockHeaderExtKey.h"

    uint8_t from_hex( char c ) {
      if( c >= '0' && c <= '9' )
          return uint8_t(c - '0');
      if( c >= 'a' && c <= 'f' )
          return uint8_t(c - 'a' + 10);
      if( c >= 'A' && c <= 'F' )
          return uint8_t(c - 'A' + 10);
      //print("from_hex: invalid hex character ", c, "\n");
      return 0;
    }

    size_t from_hex( const std::string& hex_str, char* out_data, size_t out_data_len ) {
        std::string::const_iterator i = hex_str.begin();
        uint8_t* out_pos = (uint8_t*)out_data;
        uint8_t* out_end = out_pos + out_data_len;
        while( i != hex_str.end() && out_end != out_pos ) {
          *out_pos = uint8_t(from_hex( *i ) << 4);
          ++i;
          if( i != hex_str.end() )  {
              *out_pos |= from_hex( *i );
              ++i;
          }
          ++out_pos;
        }
        return size_t(out_pos - (uint8_t*)out_data);
    }

namespace {
    block_id_type readBlockId(const std::string& s) {
        block_id_type blk_id;
        std::string blockIdStr = s.substr(0, 64);
        memcpy(blk_id.hash, blockIdStr.data(), blockIdStr.size());
        from_hex(blockIdStr, (char*)blk_id.hash, sizeof(blk_id));
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
