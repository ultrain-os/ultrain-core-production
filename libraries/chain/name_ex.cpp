#include <boost/algorithm/string.hpp>
#include <fc/exception/exception.hpp>
#include <fc/variant.hpp>
#include <ultrainio/chain/exceptions.hpp>
#include <ultrainio/chain/name_ex.hpp>

namespace ultrainio {
namespace chain {

void name_ex::set(const char* str) {
    const auto len = strnlen(str, 22);
    ULTRAIN_ASSERT(len <= 21, name_type_exception, "Name is longer than 22 characters (${name}) ",
            ("name", string(str)));
    name_ex t = string_to_name_ex(str);
    valueH = t.valueH;
    valueL = t.valueL;

    ULTRAIN_ASSERT(to_string() == string(str), name_type_exception,
            "NameEx not properly normalized (name: ${name}, normalized: ${normalized}) ",
            ("name", string(str))("normalized", to_string()));
}

// keep in sync with name::to_string() in contract definition for name
name_ex::operator string() const {
    static const char* charmaps =
            "._0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    string str(21, '.');

    uint64_t h = valueH;
    uint64_t l = valueL;

//    std::cout << std::showbase << std::hex << h << "   " << l << std::endl;
    uint64_t sym = 0;
    for (uint32_t i = 0; i <= 20; ++i) {
        if (i <= 9) {
            sym = (l & 0x3F);
            str[i] = charmaps[sym];
            l = (l >> 6);
        } else if (i == 10) {
            uint64_t rb2 = (h & 0x3);
            rb2 = (rb2 << 4);
            sym = (rb2 | l);
            str[i] = charmaps[sym];
            h = (h >> 2);
        } else {
            sym = (h & 0x3F);
            str[i] = charmaps[sym];
            h = (h >> 6);
        }
    }

    boost::algorithm::trim_right_if(str, [](char c) { return c == '.'; });
    return str;
}

} // namespace chain
} // namespace ultrainio

namespace fc {
void to_variant(const ultrainio::chain::name_ex& c, fc::variant& v) {
    v = std::string(c);
}
void from_variant(const fc::variant& v, ultrainio::chain::name_ex& check) {
    check = v.get_string();
}
} // namespace fc
