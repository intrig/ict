#pragma once
#include <sstream>
#include <iostream>

#ifdef ANDROID
#include <android/log.h>
#define IT_WARN(s) \
do { \
    std::ostringstream os; \
    os << __FILE__ << ":" <<__LINE__<< ": warning: " << s << std::endl; \
    __android_log_print(ANDROID_LOG_VERBOSE, "xenon", os.str().c_str()); \
} while (0)
#else
#define IT_WARN(s) \
do { \
    std::cerr << __FILE__ << ":" <<__LINE__<< ": warning: " << s << \
    std::endl; \
} while (0)
#endif

#define IT_THROW(desc) \
do { \
    std::ostringstream os; \
    os << desc; \
    throw ict::exception(os.str().c_str()); \
} while (0)

#define IT_PANIC(desc, ...) \
do { \
    std::ostringstream os; \
    os << desc; \
    throw ict::exception(os.str(), __FILE__, __LINE__, ##__VA_ARGS__ ); \
} while (0)

#ifndef IT_ASSERT
#define IT_ASSERT(condition) \
    do { \
    if (!(condition)) IT_PANIC(#condition); \
    } while (0)
#endif

#ifndef IT_ASSERT_MSG
#define IT_ASSERT_MSG(desc, condition) \
    do { \
    if (!(condition)) IT_PANIC(desc <<": " << #condition); \
    } while (0)
#endif

namespace ict {

class exception {
    public:
    exception(const std::string & desc, const char *src_file = "", int src_line = 0, 
        const char * xml_file = "", int xml_line = 0, int xml_column = -1) :
        desc(desc), src_file(src_file), src_line(src_line), xml_file(xml_file), xml_line(xml_line), 
        xml_column(xml_column) {}

    const char * what() const {
        std::ostringstream s;
        if (!src_file.empty()) s << "[" << src_file << ":" << src_line << "] ";
        s << desc;
        if (!xml_file.empty()) {
            s << " in [" << xml_file << ":" << xml_line;
            if (xml_column != -1) s << ":" << xml_column;
            s << "]";
        }
        what_ = s.str().c_str();
        return what_.c_str();
    }

    std::string desc;
    std::string src_file;
    int src_line;
    std::string xml_file;
    int xml_line;
    int xml_column;

    private:
    mutable std::string what_;
};
}
