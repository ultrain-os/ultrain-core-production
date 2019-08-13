#include "rpos/EvidenceFactory.h"

#include <fc/variant.hpp>
#include <fc/io/json.hpp>

#include "rpos/EvidenceMultiSign.h"
#include "rpos/EvidenceNull.h"

namespace ultrainio {
    Evidence EvidenceFactory::create(const std::string& str) {
        fc::variant v(str);
        if (!v.is_object()) {
            return EvidenceNull();
        }
        fc::variant_object o = v.get_object();
        fc::variant typeVar = o[Evidence::kType];
        int type;
        typeVar.as<int>(type);
        if (type == Evidence::kSignMultiPropose) {
            return EvidenceMultiSign(str);
        }
        return EvidenceNull();
    }
}