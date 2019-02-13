#include <ultrainio/chain/callback_manager.hpp>

#include <ultrainio/chain/callback.hpp>

namespace ultrainio { namespace chain {
    std::shared_ptr<callback_manager> callback_manager::self = nullptr;
    std::shared_ptr<callback_manager> callback_manager::get_self() {
        if (!self) {
            self = std::make_shared<callback_manager>();
        }
        return self;
    }

    void callback_manager::register_callback(std::shared_ptr<callback> cb) {
        cb_ptr = cb;
    }

    void callback_manager::unregister_callback(std::shared_ptr<callback> cb) {
        if (cb_ptr == cb) {
            cb_ptr = nullptr;
        }
    }

    std::shared_ptr<callback> callback_manager::get_callback() {
        return cb_ptr;
    }

} } // namespace ultrainio::chain