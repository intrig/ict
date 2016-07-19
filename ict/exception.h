#pragma once
#include <sstream>
#include <iostream>
#include <stdexcept>

namespace ict {
inline std::runtime_error create_exception(const std::string & desc)  {
    return std::runtime_error(desc); 
}
inline std::runtime_error create_exception(const std::string & desc, const char * file, int line)  {
    std::ostringstream os;
    os << '[' << file << ':' << line << "] " << desc;
    return std::runtime_error(os.str()); 
}
}

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

#define IT_FATAL(desc) \
do { \
    std::ostringstream os; \
    os << desc; \
    throw ict::create_exception(os.str()); \
} while (0)

#define IT_PANIC(desc) \
do { \
    std::ostringstream os; \
    os << desc; \
    throw ict::create_exception(os.str(), __FILE__, __LINE__); \
} while (0)

namespace ict {

class xml_exception : public std::exception {
    public:
    xml_exception(const std::string & desc, const char *src_file = "", int src_line = 0, 
        const char * xml_file = "", int xml_line = 0, int xml_column = -1) :
        desc(desc), src_file(src_file), src_line(src_line), xml_file(xml_file), xml_line(xml_line), 
        xml_column(xml_column) {}

    const char * what() const noexcept {
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

    mutable std::string what_;
};

#if 1
class exception : public std::exception {
    public:
    exception(const std::string & desc, const char *src_file = "", int src_line = 0, 
        const char * xml_file = "", int xml_line = 0, int xml_column = -1) :
        desc(desc), src_file(src_file), src_line(src_line), xml_file(xml_file), xml_line(xml_line), 
        xml_column(xml_column) {}

    const char * what() const noexcept {
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


    private:
    std::string desc;
    std::string src_file;
    int src_line;
    std::string xml_file;
    int xml_line;
    int xml_column;

    mutable std::string what_;
};
#endif
}
