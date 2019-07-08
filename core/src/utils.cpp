#include <core/utils.h>
#include <fc/log/logger.hpp>
#include <fc/crypto/aes.hpp>
#include <fc/crypto/hex.hpp>

namespace ultrainio {
std::vector<char> info_encode(std::string const& s) {
    std::vector<char> v(s.size());
    copy(s.begin(),s.end(),v.begin());
    std::vector<char> temp(s.size());
    for(int i =0 ;i<v.size();i++) {
        temp[i] =(~v[i])^(225-i);
    }
    return temp;
}

std::string bin2hex(const std::string& data) {
    std::vector<unsigned char> v(data.size());
    copy(data.begin(),data.end(),v.begin());
    std::vector<unsigned char> temp(data.size());
    for(int i =0 ;i<data.size();i++) {
        temp[i] =( ~data[i])^(225-i);
    }

    std::ostringstream strHex;
    strHex << std::hex << std::setfill('0');
    for (int i = 0; i < temp.size(); ++i) {
        strHex << std::setw(2)<<static_cast<unsigned int>(temp[i]);
    }
    return strHex.str();
}

std::string hex2bin(const std::string &hex) {
    std::vector<char> dest;
    auto len = hex.size();
    dest.reserve(len / 2);
    for (decltype(len) i = 0; i < len; i += 2) {
        unsigned int element;
        std::istringstream strHex(hex.substr(i, 2));
        strHex >> std::hex >> element;
        auto reverse = ~(static_cast<char>(element) ^(225-i/2));
        dest.push_back(reverse);
    }
    return std::string(dest.data(), dest.size());
}

#if 0
	std::vector<char> info_encode(std::string const& s){
		std::vector<char> v(s.size());
		copy(s.begin(),s.end(),v.begin());
		std::string key_string = "ultrain_88888888";
		fc::sha512 h = fc::sha512::hash(key_string);
		auto enc = fc::aes_encrypt(h,v);
		ilog("key ${key}",("key",h.str()));
		return enc;
}
	std::string info_s(std::string const& s){
		std::vector<char> v(s.size());
                copy(s.begin(),s.end(),v.begin());
		std::string key_string = "ultrain_88888888";
		fc::sha512 h = fc::sha512::hash(key_string);
		auto des =  fc::aes_decrypt(h,v);
		std::string out(des.begin(),des.end());
		return out;
	}
#endif

std::string aes_encode(std::string const& key, std::string const& plain) {
    fc::sha512 key512 = fc::sha512::hash(key.data(), key.length());
    std::vector<char> vc_plain(plain.length());
    std::copy(plain.begin(), plain.end(), vc_plain.begin());
    return fc::to_hex(fc::aes_encrypt(key512, vc_plain));
}

std::string aes_decode(std::string const& key, std::string const& cipher) {
    fc::sha512 key512 = fc::sha512::hash(key.data(), key.length());
    fc::string hex(cipher);
    char* buffer = new char[hex.size()];
    size_t len = fc::from_hex(hex, buffer, hex.size());
    if (len != hex.size()/2) {
        delete [] buffer;
        elog("Converting hex string fails. hex length: ${hl} result length: ${rl} cipher: ${cp}", ("hl", hex.size())("rl", len)("cp", cipher));
        return std::string();
    }

    std::vector<char> vc_cipher(len);
    std::copy(buffer, buffer + len, vc_cipher.begin()); 
    delete [] buffer;
    auto plain = fc::aes_decrypt(key512, vc_cipher);
    return std::string(plain.data(), plain.size());
}

} // end of namespace ultrainio
