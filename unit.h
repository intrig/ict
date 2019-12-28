#pragma once
#include "ict.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace ict {
enum Platform {

    MSVC = 0x000000001,
    Apple = 0x000000002,
    Linux = 0x000000004,
    Unknown = 0x000000008
};

inline Platform hostPlatform() {
#if defined(_MSC_VER)
    return MSVC;
#endif
#if defined(__APPLE__)
    return Apple;
#endif
#if defined(__linux__)
    return Linux;
#endif
    return Unknown;
}
} // namespace ict

#ifdef IT_ASSERT
#undef IT_ASSERT
#endif
#define IT_ASSERT(condition)                                                   \
    do {                                                                       \
        if (!(condition))                                                      \
            throw ict::unit_error(#condition, __FILE__, __LINE__);             \
    } while (0)

#ifdef IT_ASSERT_MSG
#undef IT_ASSERT_MSG
#endif
#define IT_ASSERT_MSG(desc, condition)                                         \
    do {                                                                       \
        if (!(condition)) {                                                    \
            std::ostringstream os_assert_msg_;                                 \
            os_assert_msg_ << desc << ": " << #condition;                      \
            throw ict::unit_error(os_assert_msg_.str(), __FILE__, __LINE__);   \
        }                                                                      \
    } while (0)

#define IT_FORCE_ASSERT(desc)                                                  \
    do {                                                                       \
        std::ostringstream os_force_assert_;                                   \
        os_force_assert_ << desc;                                              \
        throw ict::unit_error(os_force_assert_.str(), __FILE__, __LINE__);     \
    } while (0)

#define IT_WARN_MSG(desc, condition)                                           \
    do {                                                                       \
        if (!(condition)) {                                                    \
            std::ostringstream os_warn_msg_;                                   \
            os_warn_msg_ << desc << ": " << #condition;                        \
            throw unit_error(os_warn_msg_.str(), __FILE__, __LINE__, true);    \
        }                                                                      \
    } while (0)

namespace ict {

struct unit_error {
    unit_error(const std::string &desc, const char *file, int line,
               bool warn = false)
        : desc(desc), file(file), line(line), warn(warn) {}
    std::string desc;
    std::string file;
    int line;
    bool warn;
};

template <typename T> class unit_test {
  public:
    typedef void (T::*Pointer)();

    struct test {
        test(Pointer p, unsigned int mask = 0, bool iso = false)
            : p(p), mask(mask), iso(iso) {}
        Pointer p;
        unsigned int mask;
        bool iso;
    };

    unit_test(T *test) : test_(test), mask_(0), iso_(false) {
        tests.reserve(100);
        test_->register_tests(*this);
    }
    void skip(unsigned int mask = 0xFFFFFFFF) { mask_ = mask; }
    void skip(Pointer p) {
        test test(p, 0xFFFFFFFF);
        tests.push_back(test);
    }
    void cont() { mask_ = 0; }
    void add(Pointer p) {
        test test(p, mask_);
        tests.push_back(test);
    }
    void iso(Pointer p) {
        test test(p, 0, true);
        tests.push_back(test);
        iso_ = true;
    }
    int run() {
        ict::timer tmr;
        int skipped = 0;
        typename std::vector<test>::iterator it;
        tmr.start();
        for (it = tests.begin(); it != tests.end(); ++it) {
            Pointer p = it->p;
            try {
                if ((iso_ && !it->iso) || (ict::hostPlatform() & it->mask)) {
                    std::cerr << "S";
                    skipped++;
                } else {
                    std::cerr << ".";
                    (test_->*p)();
                }
            } catch (unit_error &e) {
                std::cerr << std::endl << e.file << ":" << e.line << ": ";
                if (e.warn)
                    std::cerr << "warning: ";
                else
                    std::cerr << "error: ";
                std::cerr << e.desc << std::endl;

                if (!e.warn)
                    return 1;
            } catch (std::exception &e) {
                std::cerr << "\n The unit test framework caught an unexpected "
                             "exception: "
                          << e.what() << '\n';
                return 1;
            }
        }
        tmr.stop();
        std::cerr << "\ntotal time: " << ict::to_string(tmr);

        if (skipped) {
            std::cerr << std::endl
                      << "warning: " << skipped << " test"
                      << ((skipped > 1) ? "s " : " ") << "skipped";
        }
        std::cerr << std::endl;
        return 0;
    }

  private:
    std::vector<test> tests;
    T *test_;
    unsigned int mask_;
    bool iso_;
};
} // namespace ict
