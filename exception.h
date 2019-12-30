#pragma once
#include <iostream>
#include <sstream>
#include <stdexcept>
#ifdef ANDROID
#include <android/log.h>
#endif

namespace ict {
inline std::runtime_error create_exception(const std::string &desc) {
    return std::runtime_error(desc);
}
inline std::runtime_error create_exception(const std::string &desc,
                                           const char *file, int line) {
    std::ostringstream os;
    os << '[' << file << ':' << line << "] " << desc;
    return std::runtime_error(os.str());
}
} // namespace ict

#define IT_WARN(s)                                                             \
    do {                                                                       \
        std::ostringstream os_warn_;                                           \
        os_warn_ << s;                                                         \
        std::cerr << ict::create_exception(os_warn_.str(), __FILE__, __LINE__) \
                         .what()                                               \
                  << '\n';                                                     \
    } while (0)

// Throw a std::runtime_error with description.
#define IT_FATAL(desc)                                                         \
    do {                                                                       \
        std::ostringstream os_fatal_;                                          \
        os_fatal_ << desc;                                                     \
        throw ict::create_exception(os_fatal_.str());                          \
    } while (0)

// Throw a std::runtime_error with description and source file and line number
// added.
#define IT_PANIC(desc)                                                         \
    do {                                                                       \
        std::ostringstream os_panic_;                                          \
        os_panic_ << desc;                                                     \
        throw ict::create_exception(os_panic_.str(), __FILE__, __LINE__);      \
    } while (0)
