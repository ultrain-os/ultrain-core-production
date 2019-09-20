#include "core/ExtType.h"

namespace ultrainio {
    bool ExtType::operator == (const ExtType& rhs) const {
        if (this == &rhs) {
            return true;
        }
        return key == rhs.key && value == rhs.value;
    }

    bool ExtType::operator != (const ExtType& rhs) const {
        return !this->operator==(rhs);
    }
}
