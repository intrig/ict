#pragma once
//-- Copyright 2015 Intrig
//-- see https://github.com/intrig/xenon for license
#include <cstring>
#include <string>
#include <fstream>
#include <algorithm>
#include <vector>
#include <memory>

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#if defined(_MSC_VER)
#include <direct.h>
#endif
#include <errno.h>
#include <iostream>
#include <sstream>
#include <time.h>
#include <chrono>
#include <bitset>

#include "netvar.h"
#include "exception.h"

#ifdef _MSC_VER

#pragma warning(push)
// This warning appears with std::copy and other std functions called with pointers
#pragma warning(disable : 4996) // unsafe call

#pragma warning(disable : 4005) // macro redefiniton

#endif // _MSC_VER

namespace ict {

template <typename T>
std::string to_string(const T & value)
{
#if defined(ANDROID)
    std::ostringstream os;
    os << value;
    return os.str();
#else
    //using namespace std;
    return std::to_string(value);
#endif
}

// split on a single character
template <typename T>
inline std::vector<std::string> split(const T & source, char c) {
    std::vector<std::string> l;
    if (source.size() == 0) return l;

    auto first = source.begin();
    auto last = source.end();

    while (first != last) {
        auto i = std::find(first, source.end(), c);
        l.push_back(std::string(first, i));
        first = i;
        if (first != last) ++first;
    }
    return l;
}

// split on any of the supplied characters
template <typename T>
inline std::vector<std::string> split(const T & source, const char * s, bool include_del = false) {
    std::vector<std::string> l;
    if (source.size() == 0) return l;

    auto first = 0;
    auto last = source.size();

    while (first != last) {
        auto i = source.find_first_of(s, first);
        if (i == std::string::npos) {
            auto s = source.substr(first);
            if (!s.empty()) l.push_back(s);
            first = last;
        } else {
            auto s = std::string(source.begin() + first, source.begin() + i + (include_del == true));
            l.push_back(s);
            first = i;
            ++first;
        }
    }
    return l;
}

namespace util {
template <typename T>
T read_line(T first, T last, std::string & line) {
    while (first != last && *first != '\n') {
        line += *first;
        ++first;
    }
    line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
    line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());

    if (first == last) return first;
    ++first; // skip the \n
    return first;
}

}
template <typename T>
// T is a ForwardIterator
inline std::vector<std::string> line_split(T first, T last) {
    std::vector<std::string> lines;
    std::string line;
    while (first != last) {
        first = util::read_line(first, last, line);
        lines.push_back(line);
        line.clear();
    }
    return lines;
}

inline std::string line_join(const std::vector<std::string> & v) {
    std::string x;
    for (auto & l : v) {
        x += l;
        x+= '\n';
    }
    x.pop_back();
    return x;
}


// if the delimiter occurs twice, then it is not a delimeter, it is a single char
inline std::vector<std::string> escape_split(const std::string & source, char del) {
    enum State { 
        ReadingName,
        FoundDelimiterOrEsc,
    };

    std::vector<std::string> l;
    State state = ReadingName;
    std::string curr;

    for (auto it = source.begin(); it!=source.end(); ++it) {
        switch (state) {
            case ReadingName : 
                if (*it == del) state = FoundDelimiterOrEsc;
                else curr += *it;
                break;
            case FoundDelimiterOrEsc : 
                if (*it == del) {
                    curr += del;
                } else {
                    l.push_back(curr);
                    curr.clear();
                    curr += *it;
                }
                state = ReadingName;
                break;
        }
    }
    if (!curr.empty()) l.push_back(curr);

    return l;
}


template <typename Stream, typename I>
inline void join(Stream & os, I first, I last, const std::string & del) {
    if (first == last) return;
    os << *first;
    ++first;
    while (first != last) {
        os << del << *first;
        ++first;
    }
}

template <typename I>
inline std::string join(I first, I last, const std::string & del) {
    std::ostringstream os;
    join(os, first, last, del);
    return os.str();
}

template <typename C>
inline std::string join(const C & cont, const std::string & del = "") {
    return join(cont.begin(), cont.end(), del);
}

// replace all occurances of a with b in string x.  Return x.
inline std::string & replace(std::string & x, std::string const & a, std::string const & b) {
    size_t pos = 0;
    while((pos = x.find(a, pos)) != std::string::npos) {
        x.replace(pos, a.length(), b);
        pos += b.length();
    }
    return x;
}

inline const char * spaces(int spaces)
{
    static const char s[] =  
        "                                                           "
        "                                                           "
        "                                                           "
        "                                                           ";
    static const size_t l = strlen(s);

    if (spaces < 0) spaces = 0;
    return (unsigned) spaces > l ? s : s + (l - spaces);
}

inline void make_dir(std::string const & path)
{
#if defined(_MSC_VER)
    if (_mkdir(path.c_str()) == ENOENT) IT_PANIC("path not found " << path);
#else
    int ret = mkdir(path.c_str(), S_IRWXU| S_IRGRP| S_IWGRP| S_IROTH| S_IWOTH);
    if (!ret && ret != EEXIST) IT_PANIC("cannot create directory " << path);
#endif

}

inline int string_case_compare(const char * string1, const char * string2, size_t count)
{
#if defined(_MSC_VER)
	return _strnicmp(string1, string2, count); 
#else
	return strncasecmp(string1, string2, count); 
#endif
}

inline bool string_to_int(int & value, const char * s, int base=10) {
    if (!s) return false;
    char *end;
    errno = 0;
    value = strtol(s, &end, base);
    return (errno == 0 && *end == '\0');
}

template <typename T>
inline bool string_to_int(int & value, const T & s, int base=10) {
    return string_to_int(value, s.c_str(), base);
}

inline bool string_to_int64(int64_t & value, const char * s, int base=10) {
    if (!s) return false;
    char *end;
    errno = 0;
    value = strtoll(s, &end, base);
    if (errno) IT_WARN("string_to_int64 errno: " << strerror(errno));
    return (errno == 0 && *end == '\0');
}

inline std::string ucfirst(const std::string & value) {
    if (value.empty()) return value;
    std::string v = value;
    v[0] = toupper(v[0]);
    return v;
}

inline std::string uppercase(const std::string & value) {
    std::string v = value;
    for (auto & i : v) i = toupper(i);
    return v;
}

inline bool is_binary(const std::string & value) {
    return value.find_first_not_of("01") == std::string::npos;
}

inline bool is_hex(std::string const & value)
{
    if (value.empty()) return false;
    if (value.length() & 1) return false; // not byte aligned

    for (char c : value)
    {
        if (!(
            (c >= '0' && c <= '9') ||
            (c >= 'a' && c <= 'f') ||
            (c >= 'A' && c <= 'F')
           )) return false;
    }
    return true;
}

inline bool is_absolute_path(const std::string & value) {
    std::string v = value.c_str();
    if (v.empty()) return false;

#if defined(_WIN32)
    if (v.length() <= 3) return false;
    if ((v[1] == ':') && (v[2] == '\\')) return true;
#else
    if (v[0] == '/') return true;
#endif
    return false;
}


inline bool file_exists(std::string const & filename) {
    std::ifstream ifile(filename.c_str());
    return ifile.good();
}

inline std::string filebase(const std::string & value) {
    std::string s = value;
    size_t n = value.find_last_of('.');
    return s.substr(0, n);
}

template <typename S>
inline std::string extension(const S & filename) {
    std::string f = filename;
    auto index = f.find_last_of('.');
    if (index != std::string::npos) return f.substr(index);
    return "";
}

template <typename S>
inline void extension(S & filename, const char * new_ext) {
    std::string f = filename.c_str();
    std::string n = new_ext;
    auto index = f.find_last_of('.');
    if (index != std::string::npos) f.replace(index, std::string::npos, n);
    filename = f.c_str();
}

// remove leading and trailing spaces
template <typename S>
inline S & normalize(S & v) {
	if (v.empty()) return v;
    if (!isspace(v[0]) && !isspace(v.back())) return v;
    auto first = v.begin();
    while (first !=v.end() && isspace(*first)) ++first;
    if (first == v.end()) {
        v = "";
        return v;
    }

    auto last = --v.end();
    while (isspace(*last) && last != first) --last;
    ++last;

    if (first==v.begin() && last == v.end()) return v;
    v = S(first, last);
    return v;
}

inline std::string get_env_var(const char * name)
{
#if defined(_WIN32)
    std::string result;
    char* buf = 0;
    size_t sz = 0;
    if (_dupenv_s(&buf, &sz, name) == 0)
    {
        if (buf) {
            result = buf;
            free(buf);
        }
    }
    return result;
#else
    return getenv(name) ? getenv(name) : "";
#endif
}

// remove all spaces 
template <typename S>
inline S & squash(S & value)
{
    if (value.empty()) return value;
#if 0 // this line should work, but currently doesn't on clang (cannot deduce 3rd template argument)
    value.erase(std::remove_if(value.begin(), value.end(), std::isspace), value.end());

#else // but this does
    std::ostringstream t;
    for (auto c : value) if (!isspace(c)) t << c;
    value = t.str();

#endif
    return value;
}

/** Return a string with special XML characters escaped:
 * <pre>
 * &  -> &amp;amp;
 * <  -> &amp;lt;
 * >  -> &amp;gt;
 * \  -> &amp;quot;
 * '  -> &amp;apos;
 * \\t -> &amp;tab;
 * </pre>
 */
template <typename T>
inline T & xmlize(T & value)
{
    if (value.empty()) return value;

    std::ostringstream d;
    for (auto & cs : value)
    {
        switch (cs)
        {
            case '&' : d << "&amp;"; break;
            case '<' : d << "&lt;"; break;
            case '>' : d << "&gt;"; break;
            case '\"' : d << "&quot;"; break;
            case '\'' : d << "&apos;"; break;
            case '\t' : d << "&tab;"; break;
            default : d << cs;
        }
    }
    value = d.str();
    return value;
}

#if 0
template <typename T>
inline T xmlize(T value) {
    T svalue(value);
    return xmlize(value);
}
#endif

inline bool starts_with(std::string const & src, std::string const & value)
{
    if (!value.size() || !src.size()) return false;
    return src.compare(0, value.size(), value) == 0;
}

inline bool ends_with(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

inline bool contains(const std::string & src, const char * value) {
    return src.find(value) != std::string::npos;
}

inline bool contains(const char * src, const char * value) {
    return std::string(src).find(value) != std::string::npos;
}

inline int file_size(const std::string & filename) {
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

inline std::vector<char> read_stream(std::istream & in) {
    std::vector<char> v;
    char ch;
    while (in.get(ch)) v.push_back(ch);
    return v;
}

template <typename T>
inline std::vector<char> read_file(const T & filename) {
    int sz = file_size(filename);
    if (sz == -1) IT_PANIC("cannot stat " << filename);

    std::ifstream file(filename.c_str(), std::ios::binary);
    if (!file.good()) IT_PANIC("cannot open " << filename);
    std::vector<char> contents(sz);
    file.read((char *) contents.data(), sz);

    if (file.gcount() != sz) IT_PANIC("read " <<file.gcount() << " bytes, expected " << sz);

    return contents;
}

inline std::vector<char> read_file(const char * filename) {
    return read_file(std::string(filename));
}

template <typename T>
inline void write_file(T first, T last, const std::string & name) {
    std::ofstream s(name, std::ios::out | std::ios::binary);
    if (!s) IT_PANIC("Can't open file \"" << name << "\" for writing.");
    s.write(&(*first), last - first);
}

inline void write_file(const std::vector<std::string> & lines, const std::string & name) {
    std::ofstream s(name, std::ios::out | std::ios::binary);
    for (auto & l : lines) s << l << "\n";
}

inline bool system_bigendian() {
    // from http://www.ibm.com/developerworks/aix/library/au-endianc/index.html?ca=drs- 
    // Listing 5
    const int i = 1;
    return (*(char*)&i) == 0;
}

struct url {
    url() {};

    url(const std::string & x) {
        size_t i = x.find_last_of('/');
        if (i != std::string::npos) {
            ++i;
            path = x.substr(0, i);
        } else i = 0;

        // the rest is filename and anchor
        size_t j = x.find('#', i);

        if (j != std::string::npos) file = x.substr(i, j - i);
        else file = x.substr(i);

        if (j != std::string::npos) anchor = x.substr(j);
    }

    url(const char * x) : url(std::string(x)) {}

    void str(std::ostream &os) const { 
        os << path << file << anchor;
    }
    std::string str() const { 
        std::ostringstream os;
        str(os);
        return os.str();
    }

    bool empty() const { return path.empty() && file.empty() && anchor.empty(); };

    bool is_local() const { return path.empty() && file.empty() && !anchor.empty(); }

    bool friend operator<(const url& a, const url& b) {
        return a.str() < b.str();
    }

    std::string path; // the path, inluding the trailing '/', e.g., "3GPP/"
    std::string file; // the filename, e.g. "TS-23.040.xddl"
    std::string anchor;  // the anchor, e.g., "#10.5.3.8"
};

inline bool operator==(const url & x, const url & y) {
    return (x.path == y.path) && (x.file == y.file) && (x.anchor == y.anchor);
}

inline std::ostream & operator<<(std::ostream &os, const url & x) {
    os << x.str();
    return os;
}

// create an absolute url from a base file and a relative one
inline url relative_url(url const & base, url const & x) {
    std::ostringstream os;
    os << base.path << x.path;
    url result;
    result.path = os.str();
    result.file = x.file.empty() ? base.file : x.file;
    result.anchor = x.anchor;
    return result;
}

class timer {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time, stop_time;
public:
    void start() {
        start_time = std::chrono::high_resolution_clock::now();
    }

    double stop() {
        stop_time = std::chrono::high_resolution_clock::now();
        return milli();
    }

    double nano() const {
        return double(std::chrono::duration_cast<std::chrono::nanoseconds>(stop_time - start_time).count());
    }

    double micro() const {
        return double(std::chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time).count());
    }

    double milli() const {
        return double(std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count());
    }
};

inline std::string to_string(const timer & timer) {
    std::ostringstream os;
    if (timer.milli() > 0) os << timer.milli() << " ms";
    else if (timer.micro() > 0) os << timer.micro() << " us";
    else os << timer.nano() << " ns";
    return os.str();
}

inline std::ostream & operator<<(std::ostream & os, const timer & timer) {
    os << to_string(timer);
    return os;
}


inline bool bit_is_set(unsigned char byte, int bit_index) {
    switch (bit_index) {
        case 0: return (byte & 0x80) != 0;
        case 1: return (byte & 0x40) != 0;
        case 2: return (byte & 0x20) != 0;
        case 3: return (byte & 0x10) != 0;
        case 4: return (byte & 0x08) != 0;
        case 5: return (byte & 0x04) != 0;
        case 6: return (byte & 0x02) != 0;
        case 7: return (byte & 0x01) != 0;
    }
    IT_PANIC("invalid bit index for byte: " << bit_index);
}

template <typename T> 
// T is an InputIterator
inline bool is_integer(T first, T last) {

    if (*first == '-') {
        ++first;
        if (first == last) return false;
    }

    while (first != last) {
        if (!isdigit(*first)) return false;
        ++first;
    }
    return true;
}

inline bool is_integer(const std::string & s) {
    //IT_WARN("checking (" << s << ")");
    return is_integer(s.begin(), s.end());
}

template<typename T, typename String>
T to_integer(String s) {
    std::stringstream ss(s);
    T value;
    ss >> value;
    return value;
}

// y = log2(x)
template <typename Int> 
// Int is a postive integer
Int log_2(Int x) {
    int y = 0;
    while (x >>= 1) ++y;
    return y;
}

inline int required_bits(int64_t lower, int64_t upper) {
    auto range = upper - lower;
    if (range <= 0) return 0;
    return log_2(range) + 1;
}

/** Convert a character between '0' and 'F' to 0 and 15.
    Returns -1 if input is out of range. */
inline int hex_ascii(char ch)
{
    switch (ch) {
        case '0' : return 0;
        case '1' : return 1;
        case '2' : return 2;
        case '3' : return 3;
        case '4' : return 4;
        case '5' : return 5;
        case '6' : return 6;
        case '7' : return 7;
        case '8' : return 8;
        case '9' : return 9;
        case 'a' : case 'A' : return 10;
        case 'b' : case 'B' : return 11;
        case 'c' : case 'C' : return 12;
        case 'd' : case 'D' : return 13;
        case 'e' : case 'E' : return 14;
        case 'f' : case 'F' : return 15;
    }
    return -1;
}

// Create the table used in to_bin_string below.
inline std::string bin_table() {
    std::bitset<8> n;
    std::ostringstream os;
    for (int i = 0; i < 256; ++i) {
        n = i;
        os << "\"" << n << "\", ";
        if ((i+1) % 8 == 0) os << "\n";
    }
    return os.str();
}

template <typename T>
// T is an iterator to a char
inline std::string & to_bin_string(T first, T last, std::string & dest) {
    static char const * ahhhhh[] = {
        "00000000", "00000001", "00000010", "00000011", "00000100", "00000101", "00000110", "00000111", 
        "00001000", "00001001", "00001010", "00001011", "00001100", "00001101", "00001110", "00001111", 
        "00010000", "00010001", "00010010", "00010011", "00010100", "00010101", "00010110", "00010111", 
        "00011000", "00011001", "00011010", "00011011", "00011100", "00011101", "00011110", "00011111", 
        "00100000", "00100001", "00100010", "00100011", "00100100", "00100101", "00100110", "00100111", 
        "00101000", "00101001", "00101010", "00101011", "00101100", "00101101", "00101110", "00101111", 
        "00110000", "00110001", "00110010", "00110011", "00110100", "00110101", "00110110", "00110111", 
        "00111000", "00111001", "00111010", "00111011", "00111100", "00111101", "00111110", "00111111", 
        "01000000", "01000001", "01000010", "01000011", "01000100", "01000101", "01000110", "01000111", 
        "01001000", "01001001", "01001010", "01001011", "01001100", "01001101", "01001110", "01001111", 
        "01010000", "01010001", "01010010", "01010011", "01010100", "01010101", "01010110", "01010111", 
        "01011000", "01011001", "01011010", "01011011", "01011100", "01011101", "01011110", "01011111", 
        "01100000", "01100001", "01100010", "01100011", "01100100", "01100101", "01100110", "01100111", 
        "01101000", "01101001", "01101010", "01101011", "01101100", "01101101", "01101110", "01101111", 
        "01110000", "01110001", "01110010", "01110011", "01110100", "01110101", "01110110", "01110111", 
        "01111000", "01111001", "01111010", "01111011", "01111100", "01111101", "01111110", "01111111", 
        "10000000", "10000001", "10000010", "10000011", "10000100", "10000101", "10000110", "10000111", 
        "10001000", "10001001", "10001010", "10001011", "10001100", "10001101", "10001110", "10001111", 
        "10010000", "10010001", "10010010", "10010011", "10010100", "10010101", "10010110", "10010111", 
        "10011000", "10011001", "10011010", "10011011", "10011100", "10011101", "10011110", "10011111", 
        "10100000", "10100001", "10100010", "10100011", "10100100", "10100101", "10100110", "10100111", 
        "10101000", "10101001", "10101010", "10101011", "10101100", "10101101", "10101110", "10101111", 
        "10110000", "10110001", "10110010", "10110011", "10110100", "10110101", "10110110", "10110111", 
        "10111000", "10111001", "10111010", "10111011", "10111100", "10111101", "10111110", "10111111", 
        "11000000", "11000001", "11000010", "11000011", "11000100", "11000101", "11000110", "11000111", 
        "11001000", "11001001", "11001010", "11001011", "11001100", "11001101", "11001110", "11001111", 
        "11010000", "11010001", "11010010", "11010011", "11010100", "11010101", "11010110", "11010111", 
        "11011000", "11011001", "11011010", "11011011", "11011100", "11011101", "11011110", "11011111", 
        "11100000", "11100001", "11100010", "11100011", "11100100", "11100101", "11100110", "11100111", 
        "11101000", "11101001", "11101010", "11101011", "11101100", "11101101", "11101110", "11101111", 
        "11110000", "11110001", "11110010", "11110011", "11110100", "11110101", "11110110", "11110111", 
        "11111000", "11111001", "11111010", "11111011", "11111100", "11111101", "11111110", "11111111", 
    };

    dest.reserve((last - first) * 8);
    while (first != last) {
        dest += ahhhhh[(unsigned char) *first];
        ++first;
    }
    return dest;
}

template <typename InputIterator>
inline std::string to_bin_string(InputIterator first, InputIterator last, size_t bit_size) {
    std::string dest;
    to_bin_string(first, last, dest);
    dest.resize(bit_size);
    return dest;
}

template <typename T>
inline std::string & to_hex_string(T first, T last, std::string & dest) {
    static char const *hmmmm[] = {
        "00", "01", "02", "03", "04", "05", "06", "07", // 0
        "08", "09", "0A", "0B", "0C", "0D", "0E", "0F", // 8
        "10", "11", "12", "13", "14", "15", "16", "17", // 16
        "18", "19", "1A", "1B", "1C", "1D", "1E", "1F", // 24
        "20", "21", "22", "23", "24", "25", "26", "27", // 32
        "28", "29", "2A", "2B", "2C", "2D", "2E", "2F", // 40
        "30", "31", "32", "33", "34", "35", "36", "37", // 48
        "38", "39", "3A", "3B", "3C", "3D", "3E", "3F", // 56
        "40", "41", "42", "43", "44", "45", "46", "47", // 64
        "48", "49", "4A", "4B", "4C", "4D", "4E", "4F", // 72
        "50", "51", "52", "53", "54", "55", "56", "57", // 80
        "58", "59", "5A", "5B", "5C", "5D", "5E", "5F", // 88
        "60", "61", "62", "63", "64", "65", "66", "67", // 96
        "68", "69", "6A", "6B", "6C", "6D", "6E", "6F", // 104
        "70", "71", "72", "73", "74", "75", "76", "77",
        "78", "79", "7A", "7B", "7C", "7D", "7E", "7F",
        "80", "81", "82", "83", "84", "85", "86", "87",
        "88", "89", "8A", "8B", "8C", "8D", "8E", "8F",
        "90", "91", "92", "93", "94", "95", "96", "97",
        "98", "99", "9A", "9B", "9C", "9D", "9E", "9F",
        "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7",
        "A8", "A9", "AA", "AB", "AC", "AD", "AE", "AF",
        "B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7",
        "B8", "B9", "BA", "BB", "BC", "BD", "BE", "BF",
        "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7",
        "C8", "C9", "CA", "CB", "CC", "CD", "CE", "CF",
        "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7",
        "D8", "D9", "DA", "DB", "DC", "DD", "DE", "DF",
        "E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7",
        "E8", "E9", "EA", "EB", "EC", "ED", "EE", "EF",
        "F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7",
        "F8", "F9", "FA", "FB", "FC", "FD", "FE", "FF",
    };

    dest.reserve((last - first) * 2);
    while (first != last) {
        dest += hmmmm[(unsigned char) *first];
        ++first;
    }
    return dest;
}

template <typename T>
inline std::string to_hex_string(T first, T last) {
    std::string dest;
    return to_hex_string(first, last, dest);
}

// copy a string.  If truncate false, throw an exception if size_dest is too small, else truncate.
inline int safe_str_copy(char * dest, size_t size_dest, const char * src, size_t n, bool truncate=false) {
    if (size_dest <= n) { if (truncate) n = size_dest - 1; else return 1; }
    std::copy(src, src + n, dest);
    dest[n] = '\0';
    return 0;
}

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
inline void to_json(std::string & os, const std::vector<T> & v);

inline void to_json(std::string & os, const std::string & s) {
    os += "\"";
    os += s; 
    os += "\"";
}

template <typename T, typename U>
inline void to_json(std::string & os, const T & k, const U & v) {
    to_json(os, k);
    os += " : ";
    to_json(os, v);
    os += '\n';
}

template <typename T>
inline void to_json(std::string & os, const std::vector<T> & v) {
    os += '[';
    for (auto & i : v) {
        to_json(os, i);
        os += ", ";
    }
    if (!v.empty()) os.resize(os.size() - 2);
    os += ']';

}

template <typename T>
inline std::string to_json(const std::vector<T> & v) {
    auto os = std::string();
    to_json(os, v);
    return os;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

} // namespace

