#pragma once
#include <string>

namespace ict {

// output string string with 80% functionality and 20X performance over std::ostringstream, and easier to spell.
struct osstream {
    std::string x;
    // Move the internal string to avoid copying.  The stream is in an undefined state after this.  It is meant to be
    // the last thing you do.
    std::string && take() { 
        return std::move(x);
    }
    // Copy the current string
    std::string str() const { 
        return x;
    }
    void clear() { 
        x.clear();
    }
};

namespace util {
    template <typename S, typename T>
    S & append(S & os, const T & x) {
        os.x += x;
        return os;
    }
    template <typename S, typename T>
    S & append_number(S & os, const T & x) {
        os.x += std::to_string(x);
        return os;
    }
}

inline osstream & operator<<(osstream &os, char x) { return util::append(os, x); }
inline osstream & operator<<(osstream &os, const std::string & x) { return util::append(os, x); }
inline osstream & operator<<(osstream &os, const char * x) { return util::append(os, x); }
inline osstream & operator<<(osstream &os, int x) { return util::append_number(os, x); }
inline osstream & operator<<(osstream &os, unsigned long int x) { return util::append_number(os, x); }
inline osstream & operator<<(osstream &os, long int x) { return util::append_number(os, x); }
inline osstream & operator<<(osstream &os, long long x) { return util::append_number(os, x); }

// one weird trick to make sure there are no implicit conversions
template <typename T>
inline osstream & operator<<(osstream &os, T x) = delete;

}
