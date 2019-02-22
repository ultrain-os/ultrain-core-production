#pragma once

#include <cstdlib>

namespace ultrainio {
    class Memory {
    public:
        template <class T>
        static void freeMultiDim(T** p, size_t size) {
            if (!p) {
                return;
            }
            for (int i = 0; i < size; i++) {
                free(p[i]);
            }
            free(p);
        }
    };
}