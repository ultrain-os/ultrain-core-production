#include "core/EvidenceFactory.h"

#include <fc/variant.hpp>
#include <fc/io/json.hpp>

#include "core/MultiProposeEvidence.h"
#include "core/MultiVoteEvidence.h"
#include "core/NullEvidence.h"

namespace ultrainio {
    std::shared_ptr<Evidence> EvidenceFactory::create(const std::string& str) {
        fc::variant v = fc::json::from_string(str);
        if (v.is_object()) {
            fc::variant_object o = v.get_object();
            fc::variant typeVar = o[Evidence::kType];
            if (typeVar.as_int64() == Evidence::kMultiPropose) {
                return std::make_shared<MultiProposeEvidence>(str);
            } else if (typeVar.as_int64() == Evidence::kMultiVote) {
                return std::make_shared<MultiVoteEvidence>(str);
            }
        }
        return std::make_shared<NullEvidence>();
    }
}