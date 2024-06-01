#ifndef LIBKIWI_UTIL_EXT_VIEW_H
#define LIBKIWI_UTIL_EXT_VIEW_H
#include <libkiwi/debug/kiwiAssert.h>
#include <libkiwi/k_config.h>
#include <libkiwi/k_types.h>

/**
 * @brief Boilerplate GetInstance function for ExtView singleton classes
 */
#define K_EXTVIEW_GET_INSTANCE_DEF(TSelf, baseFunc)                            \
    static TSelf& GetInstance() {                                              \
        K_ASSERT(baseFunc() != NULL);                                          \
        return *reinterpret_cast<TSelf*>(baseFunc());                          \
    }

namespace kiwi {

/**
 * @brief Extend class using only pre-existing instances
 */
template <typename T> class ExtView : public T {
private: // Do not allow object construction/destruction
    ExtView() {
        K_ASSERT(false);
    }
    ExtView(const ExtView&) {
        K_ASSERT(false);
    }
    ~ExtView() {
        K_ASSERT(false);
    }
    ExtView& operator=(const ExtView&) {
        K_ASSERT(false);
    }
};

} // namespace kiwi

#endif
