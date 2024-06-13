#ifndef LIBKIWI_CORE_CONSOLE_OUT_H
#define LIBKIWI_CORE_CONSOLE_OUT_H
#include <libkiwi/k_config.h>
#include <libkiwi/k_types.h>
#include <libkiwi/prim/kiwiString.h>
#include <libkiwi/util/kiwiNonCopyable.h>

namespace kiwi {
namespace detail {

/**
 * @brief Console output stream
 */
class ConsoleOut : private NonCopyable {
public:
    /**
     * @brief Logs output to the console
     *
     * @param rValue Value (attempted to convert to string)
     * @return ConsoleOut reference
     */
    template <typename T> const ConsoleOut& operator<<(const T& rValue) const {
        K_LOG(ToString(rValue));
        return *this;
    }
};

} // namespace detail

/**
 * @brief Global console handle
 */
extern const detail::ConsoleOut cout;

/**
 * @brief Newline character
 */
const char endl = '\n';

} // namespace kiwi

#endif
