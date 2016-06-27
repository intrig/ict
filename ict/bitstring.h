#pragma once
//-- Copyright 2015 Intrig
//-- see https://github.com/intrig/xenon for license
#include <limits.h>
#include <cassert>

#include <ict/ict.h>

namespace ict {

namespace util {

#define PREPARE_FIRST_COPY()                                      \
    do {                                                          \
    if (src_len >= (CHAR_BIT - dst_offset_modulo)) {              \
        *dst     &= reverse_mask[dst_offset_modulo];              \
        src_len -= CHAR_BIT - dst_offset_modulo;                  \
    } else {                                                      \
        *dst     &= reverse_mask[dst_offset_modulo]               \
              | reverse_mask_xor[dst_offset_modulo + src_len];    \
         c       &= reverse_mask[dst_offset_modulo + src_len];    \
        src_len = 0;                                              \
    } } while (0)

inline void bitarray_copy(const unsigned char *src_org, int src_offset, int src_len,
                    unsigned char *dst_org, int dst_offset)
{
    static const unsigned char reverse_mask[] =
        { 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };
    static const unsigned char reverse_mask_xor[] =
        { 0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 0x00 };

    if (src_len) {
        const unsigned char *src;
              unsigned char *dst;
        int                  src_offset_modulo,
                             dst_offset_modulo;

        src = src_org + (src_offset / CHAR_BIT);
        dst = dst_org + (dst_offset / CHAR_BIT);

        src_offset_modulo = src_offset % CHAR_BIT;
        dst_offset_modulo = dst_offset % CHAR_BIT;

        if (src_offset_modulo == dst_offset_modulo) {
            int              byte_len;
            int              src_len_modulo;
            if (src_offset_modulo) {
                unsigned char   c;

                c = reverse_mask_xor[dst_offset_modulo]     & *src++;

                PREPARE_FIRST_COPY();
                *dst++ |= c;
            }

            byte_len = src_len / CHAR_BIT;
            src_len_modulo = src_len % CHAR_BIT;

            if (byte_len) {
                memcpy(dst, src, byte_len);
                src += byte_len;
                dst += byte_len;
            }
            if (src_len_modulo) {
                *dst     &= reverse_mask_xor[src_len_modulo];
                *dst |= reverse_mask[src_len_modulo]     & *src;
            }
        } else {
            int             bit_diff_ls,
                            bit_diff_rs;
            int             byte_len;
            int             src_len_modulo;
            unsigned char   c;
            /*
             * Begin: Line things up on destination. 
             */
            if (src_offset_modulo > dst_offset_modulo) {
                bit_diff_ls = src_offset_modulo - dst_offset_modulo;
                bit_diff_rs = CHAR_BIT - bit_diff_ls;

                assert(dst_offset_modulo >= 0);
                c = *src++ << bit_diff_ls;
                c |= *src >> bit_diff_rs;
                c     &= reverse_mask_xor[dst_offset_modulo];
            } else {
                bit_diff_rs = dst_offset_modulo - src_offset_modulo;
                bit_diff_ls = CHAR_BIT - bit_diff_rs;

                c = *src >> bit_diff_rs     &
                    reverse_mask_xor[dst_offset_modulo];
            }
            PREPARE_FIRST_COPY();
            *dst++ |= c;

            /*
             * Middle: copy with only shifting the source. 
             */
            byte_len = src_len / CHAR_BIT;

            while (--byte_len >= 0) {
                c = *src++ << bit_diff_ls;
                c |= *src >> bit_diff_rs;
                *dst++ = c;
            }

            /*
             * End: copy the remaing bits; 
             */
            src_len_modulo = src_len % CHAR_BIT;
            if (src_len_modulo) {
                c = *src++ << bit_diff_ls;
                c |= *src >> bit_diff_rs;
                c     &= reverse_mask[src_len_modulo];

                *dst     &= reverse_mask_xor[src_len_modulo];
                *dst |= c;
            }
        }
    }
}
}

inline void bit_copy(char * dest, size_t desto, const char * src, size_t srco, size_t length) {
    util::bitarray_copy((const unsigned char *) src, srco, length, (unsigned char *) dest, desto);
}

#define IT_BYTE(buf, index) (buf + (index / 8))
#define IT_BIT_INDEX(index) (index % 8)

inline void set_bit(unsigned char * buf, unsigned index, bool val) {
    if (val) *(IT_BYTE(buf, index)) |= (1 << (7 - IT_BIT_INDEX(index))); 
    else *(IT_BYTE(buf, index)) &= ~(0x80 >> IT_BIT_INDEX(index));
}

inline bool bit(unsigned char * buf, unsigned index) {
    return (*IT_BYTE(buf, index) >> (7 - IT_BIT_INDEX(index))) & 1;
}

struct bitstring {
    typedef char * pointer;

    // Regular
    bitstring() : buffer_(0), begin_(0) {
        set_size(0);
    }

    bitstring(size_t bit_size) {
        alloc(bit_size);
        std::fill(begin(), end(), 0);
    }

    bitstring(const bitstring & a) : bitstring(a.bit_size()) {
        std::copy(a.begin(), a.end(), begin());
    }

    bitstring(bitstring && a) noexcept {
        bit_size_ = a.bit_size_;
        buffer_ = a.buffer_;
        if (local()) begin_ = (char *) &buffer_;
        else begin_ = buffer_;
        a.bit_size_ = 0;
    }

    template <typename I>
    bitstring(I first, size_t bit_size) {
        alloc(bit_size);
        std::copy(first, first + byte_size(), begin());
    }

    bitstring(const pointer first, size_t bit_size, unsigned source_offset) {
        alloc(bit_size);
        util::bitarray_copy((const unsigned char *)first, source_offset, bit_size, (unsigned char *)begin(), 0);
    }

    bitstring(const pointer first, size_t bit_size, int source_offset, int dest_offset) {
        alloc(bit_size);
        std::fill(begin(), end(), 0);

        const unsigned char * src_org = (const unsigned char *)first;
        int src_offset = source_offset;
        int src_len = bit_size - source_offset;
        unsigned char * dst_org = (unsigned char *) begin();
        int dst_offset = dest_offset;

        util::bitarray_copy(src_org, src_offset, src_len, dst_org, dst_offset);
    }

    bitstring(int base, const char * str); 

    bitstring(const char * str) {
        std::string s(str);
        ict::squash(s);

        int len = s.size();

        if ((len >= 3) && (s[0] == '#'))
        {
            *this = bitstring(16, s.c_str() + 1);
        } else if ((len >= 2) && (s[0] == '@'))
        {
            *this = bitstring(2, s.c_str() + 1);
        } else {
            *this = bitstring(16, s.c_str());
        }
    }

    bitstring(const std::string & str) : bitstring(str.c_str()) { }

    bitstring substr(size_t index, size_t len = std::numeric_limits<size_t>::max()) const {
        if (index == bit_size()) return bitstring();
        if (index > bit_size()) IT_PANIC("bitstring::substr index out of range");
        if (len > (bit_size() - index)) len = bit_size() - index;
        return bitstring(begin(), len, index);
    }

    inline bitstring& remove(size_t index, size_t len);

    ~bitstring() {
        clear();
    }

    bitstring & operator=(const bitstring & b) {
        if (this != &b) {
            resize(b.bit_size());
            std::copy(b.begin(), b.end(), begin());
        }
        return *this;
    }

    bitstring & operator=(bitstring && b) noexcept {
        if (!local()) delete buffer_;
        bit_size_ = b.bit_size_;
        buffer_ = b.buffer_;
        if (local()) begin_ = (char *) &buffer_;
        else begin_ = buffer_;

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

    friend bool operator==(const bitstring & a, const bitstring & b) {
        // possible they are equal all but for the size (e.g., @100 and @1000)
        if (a.bit_size() != b.bit_size()) return false;
        return std::equal(a.begin(), a.end(), b.begin());
    }

    friend bool operator!=(const bitstring & a, const bitstring & b) {
        return !(a == b);
    }

    bool empty() const { return bit_size() == 0; }

    pointer begin() const { return begin_; }

    pointer end() const { return begin_ + byte_size(); }

    pointer data() const { return begin(); }

    size_t byte_size() const { 
        return ((bit_size_ % 8) !=0) + bit_size_ / 8;
    }

    size_t bit_size() const { return bit_size_; }

    bool local() const { return bit_size() <= (sizeof(pointer) * 8); };

    void set(size_t index) { set_bit((unsigned char *) begin(), index, 1); }
    void reset(size_t index) { set_bit((unsigned char *) begin(), index, 0); }
    bool at(size_t index) const { return bit((unsigned char *)begin(), index); }

    void clear() {
        if (!local()) delete buffer_;
        set_size(0);
        buffer_ = 0;
        begin_ = 0;
    }

    private:

    void alloc(size_t s) {
        set_size(s);
        if (local()) {
            begin_ = (char *) &buffer_;
        } else {
            buffer_ = new char[byte_size()];
            begin_ = buffer_;
        }
        begin()[byte_size() - 1] = 0; // zero the last byte so byte compares will work
    }

    void set_size(size_t bit_size) {
        bit_size_ = bit_size;
    }

    size_t bit_size_ = 0;
    pointer buffer_;
    pointer begin_;
};

inline std::string to_string(const bitstring & bits) {
    std::string dest;
    if (!bits.bit_size()) return dest;
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

inline std::ostream& operator<<(std::ostream& os, const bitstring & bits) {
    os << to_string(bits);
    return os;
}


struct ibitstream {
    ibitstream() = delete;

    ibitstream(const ibitstream &) = delete;

    ibitstream(const bitstring & bits) : bits(bits) {
        index = 0;
        end_list.push_back(bits.bit_size());
        mark();
    }

    // read up to n bits blindly
    bitstring read_blind(size_t n) {
        index += n;
        return bitstring(bits.begin(), n, index - n);
    }

    // read up to n bits
    bitstring read(size_t n) {
        if (n > remaining()) n = remaining();
        return read_blind(n);
    }

    bitstring read_to(char ch) {
        auto first = bits.begin() + index / 8; // current byte
        auto n = 0;
        while (*first != ch) {
            IT_WARN("ch = " << *first);
            ++first;
            ++n;
        }
        ++n; // include ch 
        return read_blind(n * 8);
    }

    // peek ahead
    bitstring peek(size_t n, size_t offset=0) {
        return bitstring(bits.begin(), n, index + offset);
    }

    size_t tellg() const { return index; }

    ibitstream& seek(size_t n) { 
        index += n; 
        return *this;
    }

    void constrain(size_t length) {
        if (length > remaining()) length = remaining();
        end_list.push_back(index + length);
    }

    void unconstrain() { end_list.pop_back(); }

    size_t remaining() const { 
        return end_list.back() - index; 
     };

    void mark() {
        marker_list.push_back(index);
    }

    void unmark() { marker_list.pop_back(); }

    size_t last_mark() const {
        return marker_list.back();
    }

    bool eobits() const { return remaining() <= 0; }

    friend std::ostream& operator<<(std::ostream& os, const ibitstream & bs) {
        os << "(" << bs.index << ", " << bs.remaining() << ", " << bs.eobits() << ") " << (bs.bits);
        return os;
    }

    private:
    const bitstring & bits;
    size_t index = 0;
    std::vector<size_t> end_list;
    std::vector<size_t> marker_list;
};

// use this instead of calling ibitstream::constrain()/unconstrain() pairs.
struct constraint {
    constraint() = delete;
    constraint(const constraint &) = delete;
    constraint& operator=(const constraint &) = delete;

    constraint(ibitstream& bs, size_t length) : bs(bs) { bs.constrain(length); }
    ~constraint() { bs.unconstrain(); }

    private:
    ibitstream &bs;
};

struct bitmarker {
    bitmarker() = delete;
    bitmarker(const bitmarker &) = delete;
    bitmarker& operator=(const bitmarker &) = delete;

    bitmarker(ibitstream &bs) : bs(bs) { bs.mark(); }
    ~bitmarker() { bs.unmark(); }

    private:
    ibitstream &bs;
};

struct obitstream {
    obitstream(const bitstring & bits) : index(bits.bit_size()), data(1024) {
        std::copy(bits.begin(), bits.end(), data.begin());
    }

    obitstream() : index(0) {}

    obitstream& operator<<(const bitstring & b) {
        while (((index + b.bit_size() + 8) / 8) > data.size()) data.resize(data.size() + 1024);

        util::bitarray_copy((const unsigned char *) b.begin(), 0, b.bit_size(), (unsigned char *) &(data[0]), index);
        index += b.bit_size();
        return *this;
    }

    bitstring bits() {
        return bitstring(data.begin(), index);
    }
    int index;
    // TODO change this to a bitstring and then use move copy for bits()?
    std::vector<char> data;
};

inline bitstring::bitstring(int base, const char * str) {
    std::string s(str);
    switch (base) {
        case 2:
            if (ict::is_binary(s)) {
                resize(s.size());
                for (unsigned i = 0; i < bit_size(); ++i) {
                    set_bit((unsigned char *)begin(), i, s[(int)i] == '1');
                }
            }
            break;
        case 16:
            if (ict::is_hex(s)) {
                resize(s.size() * 4);

                for (unsigned cb = 0, i = 0; i < s.size(); i+=2, ++cb)
                {
                    char hex_char = str[(int)i];
                    char new_char = ict::hex_ascii(hex_char);
                    begin()[cb] = new_char << 4;
                    new_char = ict::hex_ascii(str[(int) (1 + i)]);
                    begin()[cb] |=  new_char;
                }
            }
            break;
        case 100 : // compat with IT::BitString's Encoding enum
        case 7 :  // ascii 7
            {
                for (size_t i = 0; i < s.length(); i++) {
                    obitstream os;
                    char ch = s[static_cast<unsigned>(i)];
                    bitstring t(&ch, 7, 1);
                    os << t;
                    *this = os.bits();
                }
            }
            break;
        case 101 : // compat with IT::BitString
        case 8 :  // ascii 8
            *this = bitstring((char *) s.c_str(), s.length() * 8);
            break;
        default:
            IT_WARN("unrecognized base: " << base);
            
    }
}

template <typename T>
inline void reverse_bytes(T & number) { std::reverse((char *) &number, (char *) &number + sizeof(T)); }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4800)
#endif

template <typename T>
inline T to_integer(bitstring const & bits, bool swap = true) {
    const size_t type_size = sizeof(T) * 8;
    T number = 0;
    if (bits.bit_size() == type_size) {
        number = *((T *) bits.begin());
        if (swap) reverse_bytes<T>(number);
        return number;
    } else if (bits.bit_size() < type_size) {
        switch (bits.bit_size()) {
            case 8 :
                return (T) *((uint8_t *) bits.begin());
                break;
            case 16 : {
                uint16_t n = *((uint16_t *) (void *) bits.begin());
                if (swap) reverse_bytes<uint16_t>(n);
                return (T) n;
                break;
            }
            case 32 :  {
                uint32_t n = *((uint32_t *) bits.begin());
                if (swap) reverse_bytes<uint32_t>(n);
                return (T) n;
                break;
            }
            case 64 :  {
                uint64_t n = *((uint64_t *) bits.begin());
                if (swap) reverse_bytes<uint64_t>(n);
                return (T) n;
                break;
            }
            default:
                // we are converting a bitstring to a bigger type.  So copy the bitstring into the last 
                // bits of the number,
                // leaving the first bits as zero.  Then swap and return.
                bit_copy((char *)&number, type_size - bits.bit_size(), bits.begin(), 0, bits.bit_size());
                if (swap) reverse_bytes<T>(number);
                return (T) number;
                break;

        }
    } else {
        //TODO  we are copying a bit string into a smaller sized.
        //IT_WARN("converting bit string of size " << bits.bit_size() << " to " << type_size << " bit number");
        // effectively we just remove the first n bits
        ibitstream bs(bits);
        return to_integer<T>(bs.seek(bits.bit_size() - type_size).read(type_size));
    }
    return 0;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

// 11011 (2, 1)
// 11
inline bitstring& bitstring::remove(size_t index, size_t len) {
    obitstream os;
    os << substr(0, index) << substr(index + len);
    *this = os.bits();
    return *this;
}

template <typename T> 
inline bitstring from_integer(T number, size_t dest_size=sizeof(T) * 8) {
    const size_t type_size = sizeof(T) * 8;

    // TODO make sure the compiler optimizes out this conditional for default dest_size
    if (dest_size == type_size) { 
        reverse_bytes(number);
        return bitstring((char *)&number, dest_size);
    } else if (dest_size < type_size) {
        // dest bitsting size is smaller than size of T, so remove leading bits
        reverse_bytes(number);
        return bitstring((char *) &number, dest_size, type_size - dest_size);
    } else {
        // dest size is greater than size of T, so pad right with 0 bits
        bitstring t(dest_size - type_size);
        std::fill(t.begin(), t.end(), 0);
        obitstream obs(t);
        obs << from_integer(number);
        return obs.bits();
    }
}

namespace util {
inline std::vector<unsigned char> unpack_bytes(std::vector<unsigned char> const & packedBytes) {
    if (packedBytes.empty()) return std::vector<unsigned char>();

    const unsigned char _decodeMask[7] = { 128, 192, 224, 240, 248, 252, 254 };
    std::vector<char> shiftedBytes((packedBytes.size() * 8) / 7, 0);
    
    int shiftOffset = 0;
    int shiftIndex = 0;

    // Shift the packed bytes to the left according to the offset (position of the byte)
    for (auto b = packedBytes.begin(); b!=packedBytes.end(); ++b) {
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

    if (shiftedBytes.size() > 0) unpackedBytes[unpackIndex - 1] = shiftedBytes[unpackIndex - 1];

    // Move the bits to the appropriate byte (unpack the bits)
    for (auto b = packedBytes.begin(); b!= packedBytes.end(); ++b) {
        if (unpackIndex != shiftedBytes.size()) {
            if (moveOffset == 7) {
                moveOffset = 0;
                unpackIndex++;
                unpackedBytes[unpackIndex - 1] = shiftedBytes[unpackIndex - 1];
            }

            if (unpackIndex != shiftedBytes.size())
            {
                // Extract the bits to be moved
                int extractedBitsByte = (packedBytes[moveIndex] & _decodeMask[moveOffset]);
                // Shift the extracted bits to the proper offset
                extractedBitsByte = (extractedBitsByte >> (7 - moveOffset));
                // Move the bits to the appropriate byte (unpack the bits)
                int movedBitsByte = (extractedBitsByte | shiftedBytes[unpackIndex]);

                unpackedBytes[unpackIndex] = (unsigned char)movedBitsByte;

                moveOffset++;
                unpackIndex++;
                moveIndex++;
            }
        }
    }

    // Remove the padding if exists
    if (unpackedBytes[unpackedBytes.size() - 1] == 0) unpackedBytes.pop_back();

    return unpackedBytes;
}

inline void map_to_ascii(std::vector<unsigned char> & unpacked) {
    for (auto it = unpacked.begin(); it!=unpacked.end(); ++it)
    {
        switch (*it)
        {
            case 0x00 : *it = '@'; break;
            case 0x02 : *it = '$'; break;
        }
    }
}

inline std::vector<unsigned char> to_uchar_array(bitstring const & bs) {
    const char * p = bs.begin();
    std::vector<unsigned char> dest(bs.bit_size() / 8);
    std::copy(p, p + bs.bit_size() / 8, dest.begin());
    return dest;
}

inline std::string calc_gsm7(const bitstring & bsp) {
    if (!bsp.empty()) {
        std::vector<unsigned char> packed = to_uchar_array(bsp);
        std::vector<unsigned char> unpacked = unpack_bytes(packed);
        map_to_ascii(unpacked);
        return std::string(unpacked.begin(), unpacked.end());
    }
    return std::string(); 
}

inline bitstring replace_bits(const bitstring & src, int index, bitstring const & bs) {
    bitstring front = src.substr(0, index);
    bitstring back;
    auto last = index + bs.bit_size();
    if (src.bit_size() > last) back = src.substr(last); 
    obitstream obs;
    obs << front << bs << back;
    auto ret = obs.bits();
    if (ret.bit_size() > src.bit_size()) return ret.substr(0, src.bit_size());
    return ret;
}

inline bitstring pad_left(bitstring const & bits, size_t width = 8) {
    if (bits.bit_size() % width) {
        bitstring bs(width - bits.bit_size() % width + bits.bit_size());
        return replace_bits(bs, width - bits.bit_size() % width, bits);
    }
    return bits;
}

inline bitstring & pad_right(bitstring & bits, size_t new_width) {
    auto more = new_width - bits.bit_size();
    if (more > 0) bits.resize(more);
    return bits;
}

} // namespace util

inline std::string gsm7(const bitstring & bits, size_t fill_bits = 0)
{
    if (fill_bits == 0) return util::calc_gsm7(bits);

    auto bs = bits;
    size_t len = fill_bits;

    // remove the fill bits
    bs.remove(8 - fill_bits, len);

    // get the first character
    auto pre = bs.substr(0, 7);
    pre = util::pad_left(pre, 8);
    auto first = gsm7(pre);

    // get the remaining string
    auto post = bs.substr(7, bs.bit_size() - 7);
    auto rem = gsm7(post);
    
    std::ostringstream os;
    os << first << rem;
    return os.str();
}

inline std::string to_bin_string(const bitstring & bits) {
    return ict::to_bin_string(bits.begin(), bits.end(), bits.bit_size());
}
}
