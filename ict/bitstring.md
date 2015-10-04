# ict::bitstring Reference

```c++
#include <ict/bitstring.h>
```

A bitstring provides a way to access arbitrarily sized data at a bit level. 

TODO: Make an `ict::bit_view` struct that represents an address and bit offset.  Use that for the low level APIs to
avoid confusion.

## Constructors

```c++
    bitstring();                        // empty bitstring
    bitstring(size_t bit_size);         // bitstring filled with 0 bits
    bitstring(const bitstring & a);     // copy constructor
    bitstring(bitstring && a) noexcept; // move constructor

    // bitstring from an input iterator and bit size.
    bitstring(Iter first, size_t bit_size);

    // bitstring from a bit_view(first, source_offset) of length bit_size
    bitstring(const pointer first, size_t bit_size, unsigned source_offset);

    // same as above, but will copy the bits into the bitstring at an offset.
    bitstring(const pointer first, size_t bit_size, int source_offset, int dest_offset);
```

## Constructing bitstrings from strings

The following table provides examples on how represent a bitstring in ascii:

string   | meaning
---------|---------
@1110    | binary 1110
#1110    | hexadecimal 1100
1110     | hexadecimal 1100
FF       | hexadecimal FF
@FF      | error! binary numbers must be 1 or 0
#F       | error! hexadecimal numbers must be byte aligned
@1 101 01  | binary 110101
AA BB CC DD | hexadecimal AABBCDEE

`c++
    bitstring(const char * str);
    bitstring(const std::string & str);

    bitstring substr(size_t index, size_t len = std::numeric_limits<size_t>::max()) const;
    inline bitstring& remove(size_t index, size_t len);

    ~bitstring();

    bitstring & operator=(const bitstring & b);

    bitstring & operator=(bitstring && b) noexcept;

    void resize(size_t s);

    friend bool operator==(const bitstring & a, const bitstring & b);

    friend bool operator!=(const bitstring & a, const bitstring & b);

    bool empty() const
    pointer begin() const
    pointer end() const
    pointer data() const

    size_t byte_size() const
    size_t bit_size() const

    bool local() const

    void set(size_t index)
    void reset(size_t index)
    bool at(size_t index) const

    void clear()


inline std::string to_string(const bitstring & bits);

inline std::ostream& operator<<(std::ostream& os, const bitstring & bits)


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

inline void reverse_bytes(Iter & number)

template <typename T>
inline T to_integer(bitstring const & bits, bool swap = true)

template <typename T> 
inline bitstring from_integer(T number, size_t dest_size=sizeof(T) * 8)

inline std::string gsm7(const bitstring & bits, size_t fill_bits = 0)

## Related functions

Copy a range of bits from one address and bit offset to another.  Set a bit, and get a bit value.
```c++
inline void bit_copy(char * dest, size_t desto, const char * src, size_t srco, size_t bit_length);
inline void set_bit(unsigned char * buf, unsigned index, bool val);
inline bool bit(unsigned char * buf, unsigned index);
```

}
