#include <core/utils.h>
namespace ultrainio {
 std::vector<char> info_encode(std::string const& s)
{
    std::vector<char> v(s.size());
    copy(s.begin(),s.end(),v.begin());
    std::vector<char> temp(s.size());
    for(int i =0 ;i<v.size();i++)
    {
	    temp[i] =(~v[i])^(225-i);
    }
    return temp;
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
}

