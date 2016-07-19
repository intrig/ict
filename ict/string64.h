#pragma once
//-- Copyright 2015 Intrig
//-- see https://github.com/intrig/xenon for license
#include <cstdint>
#include <cstring>
#include <map>
#include <iostream>
#include <ict/ict.h>

namespace ict {
class string64 {
public:
    string64() : value(0) {}
    string64(const string64 & b) : value(b.value)  {}
    string64& operator=(const string64 & b) {
        value = b.value;
        return *this;
    }
    string64(const char * b) : value(0) {
        auto n = strlen(b);
        if (n >= 8) n = 7;
        std::copy(b, b + n, c_str());
        c_str()[n] = '\0';
    }

#if defined(USE_ICT_STRING)
    string64(const IT::String & b) : string64(b.c_str()) { }
#endif
    string64(const std::string & b) : string64(b.c_str()) { }

    friend bool operator==(const string64 & a, const string64 & b) {
        return a.value == b.value;
    }

    friend bool operator!=(const string64 & a, const string64 & b) {
        return !(a == b);
    }

    // StrictWeakOrdering satisfied, so sorting works (possibly not the way you would expect :-).  
    friend bool operator<(const string64 & a, const string64 & b) {
        return a.value < b.value;
    }

    friend bool operator>(const string64 & a, const string64 & b) {
        return b < a;
    }

    friend bool operator<=(const string64 & a, const string64 & b) {
        return !(a > b);
    }

    friend bool operator>=(const string64 & a, const string64 & b) {
        return !(a < b);
    }

    char * c_str() { return (char *)&value; }
    const char * c_str() const { return (char *)&value; }
    
    char & operator[](size_t index) {
        return c_str()[index];
    }

    size_t size() const { return strlen(c_str()); }

    char * begin() { return c_str(); }
    const char * begin() const { return c_str(); }

    char * end() { return begin() + size(); }
    const char * end() const { return begin() + size(); }
    
    void clear() { value = 0; }

    bool empty() const { return value == 0; }

    bool good() const { return c_str()[7] == '\0'; }

    uint64_t value;

};

inline std::ostream & operator<<(std::ostream& os, const ict::string64 & x) {
    os << x.c_str();
    return os;
}
typedef std::vector<ict::string64> string64_list;
} // namespace ict

