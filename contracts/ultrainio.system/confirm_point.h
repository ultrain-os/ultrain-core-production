#pragma once

#include <string>
#include "ultrainiolib/block_header.hpp"
#include "block_header_ext_key.h"

    uint8_t from_hex( char c ) {
      if( c >= '0' && c <= '9' )
          return uint8_t(c - '0');
      if( c >= 'a' && c <= 'f' )
          return uint8_t(c - 'a' + 10);
      if( c >= 'A' && c <= 'F' )
          return uint8_t(c - 'A' + 10);
      //print("from_hex: invalid hex character ", c, "\n");
      ultrainio_assert(false, "wrong character in hex string");
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
    block_id_type read_block_id(const std::string& s) {
        block_id_type blk_id;
        std::string blockIdStr = s.substr(0, 64);
        from_hex(blockIdStr, (char*)blk_id.hash, sizeof(blk_id.hash));
        return blk_id;
    }
}
namespace ultrainiosystem {
    class confirm_point {
    public:
        static bool is_confirm_point(const ultrainio::block_header& header) {
            auto ext = header.header_extensions;
            for (auto& e : ext) {
                if (std::get<0>(e) == k_bls_voter_set) {
                    return true;
                }
            }
            return false;
        }

        static block_id_type get_confirmed_block_id(const ultrainio::block_header& header) {
            for (auto& e : header.header_extensions) {
                block_header_ext_key key = static_cast<block_header_ext_key>(std::get<0>(e));
                if (key == k_bls_voter_set) {
                    std::string s;
                    s.assign(std::get<1>(e).begin(), std::get<1>(e).end());
                    return read_block_id(s);
                }
            }
            return block_id_type();
        }
    };
}
