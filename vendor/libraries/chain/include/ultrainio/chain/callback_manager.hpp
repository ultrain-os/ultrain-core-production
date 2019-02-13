#pragma once

#include <memory>

#include <ultrainio/chain/callback.hpp>

namespace ultrainio { namespace chain {
    class callback;

    class callback_manager {
    public:
        static std::shared_ptr<callback_manager> get_self();

        void register_callback(std::shared_ptr<callback> cb);

        void unregister_callback(std::shared_ptr<callback> cb);

        std::shared_ptr<callback> get_callback();

    private:
        static std::shared_ptr<callback_manager> self;
        std::shared_ptr<callback> cb_ptr;
    };

} } // namespace ultrainio::chain