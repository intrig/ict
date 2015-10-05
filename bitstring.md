# Bit strings and streams

```c++
#include <ict/bitstring.h>

namespace ict
```

Bitstrings store resizable data and provide access at a bit level.  There are convenient ways of converting 
bitstrings to and from strings and integers.  The bitstring unit test provides usage examples.

Input and output bit streams can be used for writing and reading bits to and from a stream.

Example usage can be found in the unit tests: 
* [bitstringunit.cpp](unit/bitstring/bitstringunit.cpp)
* [unit/bitstring/convert.cpp]

* [ict::bitstring](#bitstring)
* [ict::ibitstream](#ibitstream)
* [ict::obitstream](#obitstream)
* [ict::Related Functions](#functions)

## <a name="bitstring"/> ict::bitstring

A bitstring provides a way to access arbitrarily sized data at a bit level. 

For API calls that use strings to encode a bitstring, the following convention is used.  Strings preceded with a `#`
denotes hexadecimal, an `@` symbol denotes binary.  Otherwise, strings are interpreted as hexadecimal.  Spaces are
ignored.

The following table provides examples on how to represent a bitstring in ASCII:

ASCII   | meaning
---------|---------
@1110    | binary 1110
#1110    | hexadecimal 1100
1110     | hexadecimal 1100
FF       | hexadecimal FF
@FF      | error! binary numbers must be 1 or 0
#F       | error! hexadecimal numbers must be byte aligned
@1 101 01   | binary 110101
AA BB CC DD | hexadecimal AABBCDEE

```c++
struct bitstring {
```

**Constructors**

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

    // create from strings
    bitstring(const char * str);
    bitstring(const std::string & str);
```
**Operators**
```c++
    bitstring & operator=(const bitstring & b);
    bitstring & operator=(bitstring && b) noexcept;
    friend bool operator==(const bitstring & a, const bitstring & b);
    friend bool operator!=(const bitstring & a, const bitstring & b);
```
**Methods**

```c++
    // return a substring
    bitstring substr(size_t index, size_t len = std::numeric_limits<size_t>::max()) const;
    inline bitstring& remove(size_t index, size_t len); // remove a substring
    void resize(size_t s);  // resize

    bool empty() const // check for empty

    pointer begin() const // return an iterator (this is a char *)
    pointer end() const
    pointer data() const // same as begin()

    size_t byte_size() const // size in bytes
    size_t bit_size() const  // size in bits

    bool local() const // denotes if the bitstring is stored locally (true if its smaller than 64 bits)

    void set(size_t index) // set a bit to 1
    void reset(size_t index) // set a bit to 0
    bool at(size_t index) const // get the bit value 

    void clear()
}
```

## <a name="ibitstream"/> ict::ibitstream

An input bit stream is modeled after the std::istream.  However it acts upon bits and not bytes.  It provides
a convenient way to read bits from a bitstring (a protocol message, for example).

*TODO: There is a lot of room for performance improvement here.  Using move semantics through for one, also the
output stream operator is not ideal.*

```c++
struct ibitstream {
```
The only way to create an `ibitstream` is to initialize its constructor with a `bitstring`.  

```c++
    ibitstream() = delete;
    ibitstream(const ibitstream &) = delete;
    ibitstream(const bitstring & bits) : bits(bits) 

    // read up n bits blindly.  Values of n greater than remaining() result in undefined behavior.
    bitstring read_blind(size_t n) 

    bitstring read(size_t n) // read up to n bits
    bitstring peek(size_t n, size_t offset=0) // peek ahead

    size_t tellg() const // return the current index

    ibitstream& seek(size_t n) // advance the index

    void constrain(size_t length) // temporarily set the remaining bits to length (reentrant)
    void unconstrain() // lift the last constraint

    void mark() // set a marker to remember current index that can be used by last_mark() (reentrant)
    void unmark() // remove last mark
    size_t last_mark() const // return the index of the last mark set

    size_t remaining() const // remaming number of bits to read 

    bool eobits() const // return if at the end
};
```

**Constraints and Marks**

Instead of using the ibitstream `constrain()` and `unconstrain()`, or `mark()` and `unmark()`, a single `constraint` or
`bitmarker` object initialized with the `ibitstream` can be created that uses RAII.

```c++
struct constraint {
    constraint() = delete;
    constraint(const constraint &) = delete;
    constraint& operator=(const constraint &) = delete;
    constraint(ibitstream& bs, size_t length) : bs(bs) { bs.constrain(length); }
    ~constraint() { bs.unconstrain(); }
};

struct bitmarker {
    bitmarker() = delete;
    bitmarker(const bitmarker &) = delete;
    bitmarker& operator=(const bitmarker &) = delete;
    bitmarker(ibitstream &bs) : bs(bs) { bs.mark(); }
    ~bitmarker() { bs.unmark(); }
};
```

## <a name="obitstream"/> ict::obitstream

Output bit streams can be used to construct bitstrings from others.

```c++
struct obitstream {
    // create a stream and initialize it with bits
    obitstream(const bitstring & bits)

    obitstream() // create obitstream
    obitstream& operator<<(const bitstring & b) // stream operator

    bitstring bits() // return contents of stream as a bitstring
};
```

## <a name="functions"/> Related Functions and Algorithms

```c++
inline void reverse_bytes(T & number)

// convert a bitstream to an integer of a given type.  Little-endian representation is assumed.
template <typename T>
inline T to_integer(bitstring const & bits, bool swap = true)

// convert a number to a bitstring
template <typename T> 
inline bitstring from_integer(T number, size_t dest_size=sizeof(T) * 8)

// text mesaging support, of course.
inline std::string gsm7(const bitstring & bits, size_t fill_bits = 0)

// Convert to std::string.  Byte aligned bitstrings will be returned in hex, otherwise binary.
inline std::string to_string(const bitstring & bits);

// Output stream operator, uses to_string() above.
inline std::ostream& operator<<(std::ostream& os, const bitstring & bits)

// Copy a range of bits from one address and bit offset to another.  This is the the thing that actually does 
// something.  The API should be improved by using array_view
inline void bit_copy(char * dest, size_t desto, const char * src, size_t srco, size_t bit_length);

// set a bit
inline void set_bit(unsigned char * buf, unsigned index, bool val);

// get a bit
inline bool bit(unsigned char * buf, unsigned index);
```

}
