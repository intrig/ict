#pragma once
#include "ict.h"
#include <cassert>
#include <limits.h>
#include <random>

namespace ict {

template <typename T, typename S> inline T *it_byte(T *buf, S &index) {
    return buf + (index / 8);
}

template <typename S> inline S it_bit_index(S &index) { return (index % 8); }

template <typename Char>
inline void set_bit(Char *buf, size_t index, bool val) {
    if (val)
        *(it_byte(buf, index)) |= (1 << (7 - it_bit_index(index)));
    else
        *(it_byte(buf, index)) &= ~(0x80 >> it_bit_index(index));
}

template <typename Char> inline bool get_bit(Char *buf, size_t index) {
    return (*it_byte(buf, index) >> (7 - it_bit_index(index))) & 1;
}

namespace detail {
// bit proxy type
struct bit_proxy {
    bit_proxy() : byte_(nullptr), bit_(0) {}
    bit_proxy(char *byte, size_t bit)
        : byte_(reinterpret_cast<unsigned char *>(byte)), bit_(bit) {}
    bit_proxy(unsigned char *byte, size_t bit) : byte_(byte), bit_(bit) {}

    bool value() const { return get_bit(byte_, bit_); }

    void value(bool x) { set_bit(byte_, bit_, x); }
    void value(const bit_proxy &x) { set_bit(byte_, bit_, x.value()); }

    void increment() {
        ++bit_;
        normalize();
    }

    void increment(size_t n) {
        bit_ += n;
        normalize();
    }

    void decrement() {
        if (bit_)
            --bit_;
        else {
            bit_ = 7;
            --byte_;
        }
    }

    void decrement(size_t n) {
        // TODO implement as O(1)
        for (size_t i = 0; i < n; ++i)
            decrement();
    }

    std::ptrdiff_t difference(const bit_proxy &b) const {
        auto bytes = byte_ - b.byte_;
        auto bits = bit_ - b.bit_;
        return (bytes * 8) + bits;
    }

    bool identical(const bit_proxy &b) const {
        normalize();
        b.normalize();
        return byte_ == b.byte_ && bit_ == b.bit_;
    }

    unsigned char *get_byte() {
        normalize();
        return byte_;
    }

    const unsigned char *get_byte() const {
        normalize();
        return byte_;
    }

    unsigned char *get_unsigned_byte() { return get_byte(); }

    const unsigned char *get_unsigned_byte() const {
        return const_cast<unsigned char *>(get_byte());
    }

    size_t bit() const { return bit_; }

  private:
    void normalize() const {
        if (bit_ > 7) {
            byte_ += bit_ / 8;
            bit_ = bit_ % 8;
        }
    }

    mutable unsigned char *byte_;
    mutable size_t bit_;
};

template <bool is_const> struct bit_iterator_base {
    typedef std::ptrdiff_t difference_type;
    typedef bit_proxy value_type;

    typedef
        typename std::conditional<is_const, const bit_proxy, bit_proxy>::type
            proxy_type;

    typedef proxy_type *pointer;
    typedef proxy_type &reference;
    typedef std::bidirectional_iterator_tag iterator_category;

    bit_iterator_base() {}
    bit_iterator_base(const bit_iterator_base<false> &b) : value(b.value) {}
    bit_iterator_base(const bit_iterator_base<true> &b) : value(b.value) {}

    bit_iterator_base(char *p, size_t b = 0) : value(p, b) {}
    bit_iterator_base(unsigned char *p, size_t b = 0)
        : value(reinterpret_cast<char *>(p), b) {}

    bit_iterator_base<false> &operator=(const bit_iterator_base<false> &b) {
        value = b.value;
        return *this;
    }

    // These aren't const because we are actually returning the proxy object.
    reference operator*() { return value; }
    pointer operator->() { return &value; }

    bit_iterator_base &operator++() {
        value.increment();
        return *this;
    }

    bit_iterator_base operator++(int) {
        bit_iterator_base tmp = *this;
        ++*this;
        return tmp;
    }

    bit_iterator_base &operator--() {
        value.decrement();
        return *this;
    }

    bit_iterator_base operator--(int) {
        bit_iterator_base tmp = *this;
        --*this;
        return tmp;
    }

    bit_iterator_base &operator+=(size_t n) {
        value.increment(n);
        return *this;
    }

    bit_iterator_base &operator-=(size_t n) {
        value.decrement(n);
        return *this;
    }

    friend bit_iterator_base operator+(bit_iterator_base x, size_t n) {
        return x += n;
    }
    friend bit_iterator_base operator+(size_t n, bit_iterator_base x) {
        return x += n;
    }
    friend bit_iterator_base operator-(bit_iterator_base x, size_t n) {
        return x -= n;
    }
    friend bit_iterator_base operator-(size_t n, bit_iterator_base x) {
        return x -= n;
    }

    friend difference_type operator-(bit_iterator_base a, bit_iterator_base b) {
        return a.value.difference(b.value);
    }

    bit_proxy operator[](size_t n) const { return *(*this + n); }

    friend bool operator==(const bit_iterator_base &a,
                           const bit_iterator_base &b) {
        return a.value.identical(b.value);
    }

    friend bool operator!=(const bit_iterator_base &a,
                           const bit_iterator_base &b) {
        return !(a == b);
    }

    friend bool operator<(const bit_iterator_base &a,
                          const bit_iterator_base &b) {
        return b - a > 0;
    }
    friend bool operator>(const bit_iterator_base &a,
                          const bit_iterator_base &b) {
        return b < a;
    }

    friend bool operator<=(const bit_iterator_base &a,
                           const bit_iterator_base &b) {
        return !(b < a);
    }
    friend bool operator>=(const bit_iterator_base &a,
                           const bit_iterator_base &b) {
        return !(a < b);
    }
    bit_proxy value;
};

// this was once a macro
template <typename A, typename B, typename C, typename D, typename E,
          typename F>
inline void prepare_first_copy(A &src_len, B const dst_offset_modulo, C *dst,
                               D const &reverse_mask, E const &reverse_mask_xor,
                               F &c) {
    if (src_len >= static_cast<size_t>(CHAR_BIT - dst_offset_modulo)) {
        *dst &= reverse_mask[dst_offset_modulo];
        src_len -= CHAR_BIT - dst_offset_modulo;
    } else {
        *dst &= reverse_mask[dst_offset_modulo] |
                reverse_mask_xor[dst_offset_modulo + src_len];
        c &= reverse_mask[dst_offset_modulo + src_len];
        src_len = 0;
    }
}

} // namespace detail

template <typename Input, typename Output>
inline void bit_copy_n(Input first, size_t bit_count, Output result) {
    typedef unsigned char value_type;

    auto src_org = first->get_unsigned_byte();
    auto dst_org = result->get_unsigned_byte();

    size_t src_offset = first->bit();
    size_t dst_offset = result->bit();
    static const unsigned char reverse_mask[] = {0x00, 0x80, 0xc0, 0xe0, 0xf0,
                                                 0xf8, 0xfc, 0xfe, 0xff};
    static const unsigned char reverse_mask_xor[] = {
        0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 0x00};

    if (bit_count) {
        const value_type *src;
        value_type *dst;
        size_t src_offset_modulo, dst_offset_modulo;

        src = src_org + (src_offset / CHAR_BIT);
        dst = dst_org + (dst_offset / CHAR_BIT);

        src_offset_modulo = src_offset % CHAR_BIT;
        dst_offset_modulo = dst_offset % CHAR_BIT;

        if (src_offset_modulo == dst_offset_modulo) {
            size_t byte_len;
            size_t src_len_modulo;
            if (src_offset_modulo) {
                value_type c;

                c = reverse_mask_xor[dst_offset_modulo] & *src++;

                detail::prepare_first_copy(bit_count, dst_offset_modulo, dst,
                                           reverse_mask, reverse_mask_xor, c);
                *dst++ |= c;
            }

            byte_len = bit_count / CHAR_BIT;
            src_len_modulo = bit_count % CHAR_BIT;

            if (byte_len) {
                memcpy(dst, src, byte_len);
                src += byte_len;
                dst += byte_len;
            }
            if (src_len_modulo) {
                *dst &= reverse_mask_xor[src_len_modulo];
                *dst |= reverse_mask[src_len_modulo] & *src;
            }
        } else {
            size_t bit_diff_ls, bit_diff_rs;
            size_t byte_len;
            size_t src_len_modulo;
            value_type c;
            /*
             * Begin: Line things up on destination.
             */
            if (src_offset_modulo > dst_offset_modulo) {
                bit_diff_ls = src_offset_modulo - dst_offset_modulo;
                bit_diff_rs = CHAR_BIT - bit_diff_ls;

                // assert(dst_offset_modulo >= 0);
                c = static_cast<value_type>(*src++ << bit_diff_ls);
                c |= *src >> bit_diff_rs;
                c &= reverse_mask_xor[dst_offset_modulo];
            } else {
                bit_diff_rs = dst_offset_modulo - src_offset_modulo;
                bit_diff_ls = CHAR_BIT - bit_diff_rs;

                c = *src >> bit_diff_rs & reverse_mask_xor[dst_offset_modulo];
            }
            detail::prepare_first_copy(bit_count, dst_offset_modulo, dst,
                                       reverse_mask, reverse_mask_xor, c);
            *dst++ |= c;

            /*
             * Middle: copy with only shifting the source.
             */
            byte_len = bit_count / CHAR_BIT;
            for (size_t i = 0; i < byte_len; ++i) {
                c = static_cast<value_type>(*src++ << bit_diff_ls);
                c |= *src >> bit_diff_rs;
                *dst++ = c;
            }
            byte_len = 0;

            /*
             * End: copy the remaing bits;
             */
            src_len_modulo = bit_count % CHAR_BIT;
            if (src_len_modulo) {
                c = static_cast<value_type>(*src++ << bit_diff_ls);
                c |= *src >> bit_diff_rs;
                c &= reverse_mask[src_len_modulo];

                *dst &= reverse_mask_xor[src_len_modulo];
                *dst |= c;
            }
        }
    }
}

// no return iterator for performance reasons
template <typename Input, typename Output>
inline void bit_copy(Input first, Input last, Output result) {
    bit_copy_n(first, last - first, result);
}

typedef detail::bit_iterator_base<false> bit_iterator;
typedef detail::bit_iterator_base<true> const_bit_iterator;

struct bitstring {
    typedef unsigned char *pointer;
    typedef const char *const_pointer;
    typedef unsigned char *iterator;
    typedef const unsigned char *const_iterator;

    // Regular
    bitstring() : buffer_(nullptr), begin_(nullptr) { set_size(0); }

    bitstring(size_t bit_size) {
        alloc(bit_size);
        std::fill(begin(), end(), 0);
    }

    bitstring(const bitstring &a) : bitstring(a.bit_size()) {
        std::copy(a.begin(), a.end(), begin());
    }

    template <typename Input> bitstring(Input first, Input last) {
        alloc(last - first);
        bit_copy(first, last, bit_begin());
    }

    template <typename Input> bitstring(Input first, size_t len) {
        alloc(len);
        bit_copy(first, first + len, bit_begin());
    }

    bitstring(bitstring &&a) noexcept {
        bit_size_ = a.bit_size_;
        buffer_ = a.buffer_;
        if (local())
            begin_ = reinterpret_cast<pointer>(&buffer_);
        else
            begin_ = buffer_;
        a.bit_size_ = 0;
    }

    bitstring(int base, const char *str);

    bitstring(const std::string &str) {
        std::string s(str);
        ict::squash(s);

        size_t len = s.size();

        if ((len >= 3) && (s[0] == '#')) {
            *this = bitstring(16, s.c_str() + 1);
        } else if ((len >= 2) && (s[0] == '@')) {
            *this = bitstring(2, s.c_str() + 1);
        } else {
            *this = bitstring(16, s.c_str());
        }
    }

    bitstring(const char *str) : bitstring(std::string(str)) {}

    bitstring substr(size_t index,
                     size_t len = std::numeric_limits<size_t>::max()) const {
        if (index == bit_size())
            return bitstring();
        if (index > bit_size())
            IT_PANIC("bitstring::substr index out of range");
        if (len > (bit_size() - index))
            len = bit_size() - index;
        return bitstring(bit_begin() + index, bit_begin() + index + len);
    }

    inline bitstring &remove(size_t index, size_t len);

    ~bitstring() { clear(); }

    bitstring &operator=(const bitstring &b) {
        if (this != &b) {
            resize(b.bit_size());
            std::copy(b.begin(), b.end(), begin());
        }
        return *this;
    }

    bitstring &operator=(bitstring &&b) noexcept {
        if (!local())
            delete[] buffer_;
        bit_size_ = b.bit_size_;
        buffer_ = b.buffer_;
        if (local())
            begin_ = reinterpret_cast<pointer>(&buffer_);
        else
            begin_ = buffer_;

        b.bit_size_ = 0;
        return *this;
    }

    void resize(size_t s) {
        if (s > bit_size_) {
            auto bs = bitstring(s);
            std::copy(begin(), end(), bs.begin());
            *this = std::move(bs);
        } else {
            bit_size_ = s;
        }
    }

    friend bool operator==(const bitstring &a, const bitstring &b) {
        // possible they are equal all but for the size (e.g., @100 and @1000)
        if (a.bit_size() != b.bit_size())
            return false;
        return std::equal(a.begin(), a.end(), b.begin());
    }

    friend bool operator!=(const bitstring &a, const bitstring &b) {
        return !(a == b);
    }

    bool empty() const { return bit_size() == 0; }

    iterator begin() { return begin_; }
    const_iterator begin() const { return begin_; }
    iterator end() { return begin_ + byte_size(); }
    const_iterator end() const { return begin_ + byte_size(); }

    char *data() const { return reinterpret_cast<char *>(begin_); }

    bit_iterator bit_begin() { return bit_iterator(data(), 0); }
    const_bit_iterator bit_begin() const {
        return const_bit_iterator(data(), 0);
    }
    bit_iterator bit_end() { return bit_iterator(data(), bit_size()); }
    const_bit_iterator bit_end() const {
        return const_bit_iterator(data(), bit_size());
    }

    size_t byte_size() const { return ((bit_size_ % 8) != 0) + bit_size_ / 8; }

    size_t bit_size() const { return bit_size_; }

    bool local() const { return bit_size() <= (sizeof(pointer) * 8); }

    void set(size_t index) {
        set_bit(reinterpret_cast<unsigned char *>(data()), index, 1);
    }
    void reset(size_t index) {
        set_bit(reinterpret_cast<unsigned char *>(data()), index, 0);
    }
    bool at(size_t index) const {
        return get_bit(reinterpret_cast<unsigned char *>(data()), index);
    }

    void clear() {
        if (!local())
            delete[] buffer_;
        set_size(0);
        buffer_ = nullptr;
        begin_ = nullptr;
    }

  private:
    void alloc(size_t s) {
        set_size(s);
        if (local()) {
            begin_ = reinterpret_cast<pointer>(&buffer_);
        } else {
            buffer_ = new unsigned char[byte_size()];
            begin_ = buffer_;
        }
        data()[byte_size() - 1] =
            0; // zero the last byte so byte compares will work
    }

    void set_size(size_t bit_size) { bit_size_ = bit_size; }

    size_t bit_size_ = 0;
    pointer buffer_ = nullptr;
    pointer begin_ = nullptr;
};

inline std::string to_string(const bitstring &bits) {
    std::string dest;
    if (!bits.bit_size())
        return dest;
    if (bits.bit_size() % 8) {
        dest.reserve(bits.byte_size() + 1);
        dest += '@';
        ict::to_bin_string(bits.begin(), bits.end(), dest);
        dest.resize(bits.bit_size() + 1);
    } else {
        dest.reserve((bits.end() - bits.begin()) * 2 + 1);
        dest += '#';
        ict::to_hex_string(bits.begin(), bits.end(), dest);
    }
    return dest;
}

inline std::ostream &operator<<(std::ostream &os, const bitstring &bits) {
    os << to_string(bits);
    return os;
}

struct ibitstream {
    ibitstream() = delete;

    ibitstream(const ibitstream &) = delete;

    ibitstream(const bitstring &bits_)
        : bits(bits_), bit_index(bits_.bit_begin()) {
        // index = 0;
        end_bit_list.push_back(bits.bit_end());
        mark();
    }

    void advance() {
        // ++index;
        ++bit_index;
    }
    void advance(size_t n) {
        // index += n;
        bit_index += n;
    }

    // read up to n bits blindly
    bitstring read_blind(size_t n) {
        auto f = bit_index;
        advance(n);
        return bitstring(f, n);
    }

    // read up to n bits
    bitstring read(size_t n) {
        if (n > remaining())
            n = remaining();
        return read_blind(n);
    }

    // TODO untested read_to for Dave's protocol
    bitstring read_to(char ch) {
        auto first = bit_index->get_byte();
        auto last = first;
        while (*last != ch)
            ++last;
        ++last;
        return read_blind(last - first);
    }

    // peek ahead
    bitstring peek(size_t len, size_t offset = 0) {
        return bitstring(bit_index + offset, len);
    }

    size_t tellg() const { return bit_index - bits.bit_begin(); }

    ibitstream &seek(size_t n) {
        advance(n);
        return *this;
    }

    void constrain(size_t length) {
        if (length > remaining())
            length = remaining();
        end_bit_list.push_back(bit_index + length);
    }

    void unconstrain() { end_bit_list.pop_back(); }

    size_t remaining() const { return end_bit_list.back() - bit_index; }

    void mark() { marker_bit_list.push_back(bit_index); }

    void unmark() { marker_bit_list.pop_back(); }

    size_t last_mark() const {
        return marker_bit_list.back() - bits.bit_begin();
    }

    bool eobits() const { return remaining() <= 0; }

    friend std::ostream &operator<<(std::ostream &os, const ibitstream &bs) {
        os << "(" << (bs.bit_index - bs.bits.bit_begin()) << ", "
           << bs.remaining() << ", " << bs.eobits() << ") " << (bs.bits);
        return os;
    }

  private:
    const bitstring &bits;
    const_bit_iterator bit_index;
    std::vector<const_bit_iterator> end_bit_list;
    std::vector<const_bit_iterator> marker_bit_list;
};

// use this instead of calling ibitstream::constrain()/unconstrain() pairs.
struct constraint {
    constraint() = delete;
    constraint(const constraint &) = delete;
    constraint &operator=(const constraint &) = delete;

    constraint(ibitstream &bs, size_t length) : bs(bs) { bs.constrain(length); }
    ~constraint() { bs.unconstrain(); }

  private:
    ibitstream &bs;
};

struct bitmarker {
    bitmarker() = delete;
    bitmarker(const bitmarker &) = delete;
    bitmarker &operator=(const bitmarker &) = delete;

    bitmarker(ibitstream &bs) : bs(bs) { bs.mark(); }
    ~bitmarker() { bs.unmark(); }

  private:
    ibitstream &bs;
};

struct obitstream {
    obitstream(const bitstring &bits) : index(bits.bit_size()), data(1024) {
        std::copy(bits.begin(), bits.end(), data.begin());
    }

    obitstream() : index(0) {}

    obitstream &operator<<(const bitstring &b) {
        while (((index + b.bit_size() + 8) / 8) > data.size())
            data.resize(data.size() + 1024);
        auto dest = bit_iterator(&(data[0]), index);
        bit_copy(b.bit_begin(), b.bit_end(), dest);
        index += b.bit_size();
        return *this;
    }

    bitstring bits() {
        auto first = const_bit_iterator(&data[0]);
        return bitstring(first, first + index);
    }
    size_t index;
    // TODO change this to a bitstring and then use move copy for bits()?
    std::vector<char> data;
};

template <typename T> bitstring from_ascii7(T first, T last) {
    obitstream os;
    while (first != last) {
        auto i = bit_iterator(&(*first));
        i++;
        auto bs = bitstring(i, 7);
        os << bs;
    }
    return os.bits();
}

inline bitstring::bitstring(int base, const char *str) {
    std::string s(str);
    switch (base) {
    case 2:
        if (ict::is_binary(s)) {
            resize(s.size());
            for (unsigned i = 0; i < bit_size(); ++i) {
                set_bit(reinterpret_cast<unsigned char *>(begin()), i,
                        s[i] == '1');
            }
        }
        break;
    case 16:
        if (ict::is_hex(s)) {
            resize(s.size() * 4);

            for (unsigned cb = 0, i = 0; i < s.size(); i += 2, ++cb) {
                char hex_char = str[i];
                char new_char = static_cast<char>(ict::hex_ascii(hex_char));
                data()[cb] = static_cast<char>(new_char << 4);
                new_char = static_cast<char>(ict::hex_ascii(str[1 + i]));
                data()[cb] |= new_char;
            }
        }
        break;
    case 7: // ascii 7
    {
        *this = from_ascii7(s.begin(), s.end());
    } break;
    case 8: // ascii 8
    {
        auto first = const_bit_iterator(const_cast<char *>(s.c_str()));
        *this = bitstring(first, first + s.length() * 8);
    } break;
    default:
        IT_WARN("unrecognized base: " << base);
    }
}

template <typename T> inline void reverse_bytes(T &number) {
    std::reverse(reinterpret_cast<char *>(&number),
                 reinterpret_cast<char *>(&number) + sizeof(T));
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4800)
#endif

namespace detail {
    template <typename T>
    auto convert_to_int(void *first, bool swap) -> T {
        T number = *reinterpret_cast<T *>(first);
        if (swap)
            reverse_bytes<T>(number);
        return static_cast<T>(number);
    }

}

template <typename T>
inline T to_integer(bitstring const &bits, bool swap = true) {
    const size_t type_size = sizeof(T) * 8;
    auto first = const_cast<bitstring::pointer>(bits.begin());
    T number = 0;
    if (bits.bit_size() == type_size) {
        return detail::convert_to_int<T>(first, swap);
    } else if (bits.bit_size() < type_size) {
        switch (bits.bit_size()) {
        case 8:
            return static_cast<T>(*first);
            break;
        case 16: {
            uint16_t n = *reinterpret_cast<uint16_t *>(first);
            if (swap)
                reverse_bytes<uint16_t>(n);
            return static_cast<T>(n);
            break;
        }
        case 32: {
            uint32_t n = *((uint32_t *)bits.begin());
            if (swap)
                reverse_bytes<uint32_t>(n);
            return (T)n;
            break;
        }
        case 64: {
            uint64_t n = *((uint64_t *)bits.begin());
            if (swap)
                reverse_bytes<uint64_t>(n);
            return (T)n;
            break;
        }
        default:
            // we are converting a bitstring to a bigger type.  So copy the
            // bitstring into the last bits of the number, leaving the first
            // bits as zero.  Then swap and return.
            auto dest =
                bit_iterator((char *)&number, type_size - bits.bit_size());
            bit_copy(bits.bit_begin(), bits.bit_end(), dest);
            if (swap)
                reverse_bytes<T>(number);
            return (T)number;
            break;
        }
    } else {
        // TODO  we are copying a bit string into a smaller sized.
        // IT_WARN("converting bit string of size " << bits.bit_size() << " to "
        // << type_size << " bit number");
        // effectively we just remove the first n bits
        ibitstream bs(bits);
        return to_integer<T>(
            bs.seek(bits.bit_size() - type_size).read(type_size));
    }
    return 0;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

// 11011 (2, 1)
// 11
inline bitstring &bitstring::remove(size_t index, size_t len) {
    obitstream os;
    if (index > 0)
        os << bitstring(bit_begin(), index);
    os << bitstring(bit_begin() + index + len, bit_end());
    *this = os.bits();
    return *this;
}

template <typename T>
inline bitstring from_integer(T number, size_t dest_size = sizeof(T) * 8) {
    const size_t type_size = sizeof(T) * 8;

    // TODO make sure the compiler optimizes out this conditional for default
    // dest_size
    if (dest_size == type_size) {
        reverse_bytes(number);
        auto f = bit_iterator((char *)&number);
        return bitstring(f, f + dest_size);
    } else if (dest_size < type_size) {
        // dest bitsting size is smaller than size of T, so remove leading bits
        reverse_bytes(number);
        return bitstring(bit_iterator((char *)&number) + type_size -
                             dest_size,
                         dest_size);
    } else {
        // dest size is greater than size of T, so pad right with 0 bits
        bitstring t(dest_size - type_size);
        std::fill(t.begin(), t.end(), 0);
        obitstream obs(t);
        obs << from_integer(number);
        return obs.bits();
    }
}

namespace detail {
inline std::vector<unsigned char>
unpack_bytes(std::vector<unsigned char> const &packedBytes) {
    if (packedBytes.empty())
        return std::vector<unsigned char>();

    const unsigned char _decodeMask[7] = {128, 192, 224, 240, 248, 252, 254};
    std::vector<char> shiftedBytes((packedBytes.size() * 8) / 7, 0);

    int shiftOffset = 0;
    int shiftIndex = 0;

    // Shift the packed bytes to the left according to the offset (position of
    // the byte)
    for (auto b = packedBytes.begin(); b != packedBytes.end(); ++b) {
        if (shiftOffset == 7) {
            shiftedBytes[shiftIndex] = 0;
            shiftOffset = 0;
            shiftIndex++;
        }

        shiftedBytes[shiftIndex] = (unsigned char)((*b << shiftOffset) & 127);

        shiftOffset++;
        shiftIndex++;
    }

    unsigned moveOffset = 0;
    unsigned moveIndex = 0;
    unsigned unpackIndex = 1;
    std::vector<unsigned char> unpackedBytes(shiftedBytes.size(), 0);

    if (shiftedBytes.size() > 0)
        unpackedBytes[unpackIndex - 1] = shiftedBytes[unpackIndex - 1];

    // Move the bits to the appropriate byte (unpack the bits)
    for (auto b = packedBytes.begin(); b != packedBytes.end(); ++b) {
        if (unpackIndex != shiftedBytes.size()) {
            if (moveOffset == 7) {
                moveOffset = 0;
                unpackIndex++;
                unpackedBytes[unpackIndex - 1] = shiftedBytes[unpackIndex - 1];
            }

            if (unpackIndex != shiftedBytes.size()) {
                // Extract the bits to be moved
                int extractedBitsByte =
                    (packedBytes[moveIndex] & _decodeMask[moveOffset]);
                // Shift the extracted bits to the proper offset
                extractedBitsByte = (extractedBitsByte >> (7 - moveOffset));
                // Move the bits to the appropriate byte (unpack the bits)
                int movedBitsByte =
                    (extractedBitsByte | shiftedBytes[unpackIndex]);

                unpackedBytes[unpackIndex] = (unsigned char)movedBitsByte;

                moveOffset++;
                unpackIndex++;
                moveIndex++;
            }
        }
    }

    // Remove the padding if exists
    if (unpackedBytes[unpackedBytes.size() - 1] == 0)
        unpackedBytes.pop_back();

    return unpackedBytes;
}

inline void map_to_ascii(std::vector<unsigned char> &unpacked) {
    for (auto it = unpacked.begin(); it != unpacked.end(); ++it) {
        switch (*it) {
        case 0x00:
            *it = '@';
            break;
        case 0x02:
            *it = '$';
            break;
        }
    }
}

inline std::vector<unsigned char> to_uchar_array(bitstring const &bs) {
    auto p = bs.begin();
    std::vector<unsigned char> dest(bs.bit_size() / 8);
    std::copy(p, p + bs.bit_size() / 8, dest.begin());
    return dest;
}

inline std::string calc_gsm7(const bitstring &bsp) {
    if (!bsp.empty()) {
        std::vector<unsigned char> packed = to_uchar_array(bsp);
        std::vector<unsigned char> unpacked = unpack_bytes(packed);
        map_to_ascii(unpacked);
        return std::string(unpacked.begin(), unpacked.end());
    }
    return std::string();
}

// Replace the bits starting at index with bs
inline bitstring &replace_bits(bitstring &src, size_t index,
                               bitstring const &bs) {
    const_bit_iterator first = bs.bit_begin();
    const_bit_iterator last = bs.bit_end();
    bit_iterator dest = src.bit_begin() + index;
    bit_copy(first, last, dest);
    return src;
}

inline bitstring pad_left(bitstring const &bits, size_t width = 8) {
    if (bits.bit_size() % width) {
        bitstring bs(width - bits.bit_size() % width + bits.bit_size());
        return replace_bits(bs, width - bits.bit_size() % width, bits);
    }
    return bits;
}

inline bitstring &pad_right(bitstring &bits, size_t new_width) {
    auto more = new_width - bits.bit_size();
    if (more > 0)
        bits.resize(more);
    return bits;
}
} // namespace detail

inline bitstring random_bitstring(size_t bit_len) {
    std::random_device engine;
    auto bytes = bit_len / 8 + 1;
    auto v = std::vector<unsigned char>(bytes);
    for (auto &b : v)
        b = engine();
    return ict::bitstring(bit_iterator(v.data()), bit_len);
}

inline std::string gsm7(const bitstring &bits, size_t fill_bits = 0) {
    if (fill_bits == 0)
        return detail::calc_gsm7(bits);

    auto bs = bits;
    size_t len = fill_bits;

    // remove the fill bits
    bs.remove(8 - fill_bits, len);

    // get the first character
    auto pre = bitstring(bs.bit_begin(), 7);
    pre = detail::pad_left(pre, 8);
    auto first = gsm7(pre);

    // get the remaining string
    auto post = bs.substr(7, bs.bit_size() - 7);
    auto rem = gsm7(post);

    std::ostringstream os;
    os << first << rem;
    return os.str();
}

inline std::string to_bin_string(const bitstring &bits) {
    return ict::to_bin_string(bits.begin(), bits.end(), bits.bit_size());
}
} // namespace ict
