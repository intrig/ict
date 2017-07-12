#  Bit string Tutorial and Reference
```c++
#include <ict/bitstring.h>
namespace ict

```


* 1 [Introduction](#Introduction)
* 2 [bit_iterator](#bit_iterator)
* 3 [ict::bitstring](#ict::bitstring)
    * 3.1 [Constructors](#Constructors)
    * 3.2 [Methods](#Methods)
* 4 [ict::ibitstream](#ict::ibitstream)
    * 4.1 [Constraints and Marks](#Constraints-and-Marks)
* 5 [ict::obitstream](#ict::obitstream)
* 6 [Functions](#Functions)
    * 6.1 [reverse_bytes](#reverse_bytes)
    * 6.2 [to_integer](#to_integer)
    * 6.3 [from_integer](#from_integer)
    * 6.4 [gsm7](#gsm7)
    * 6.5 [to_string](#to_string)
    * 6.6 [operator<<](#operator<<)
    * 6.7 [set_bit](#set_bit)
    * 6.8 [bit](#bit)
    * 6.9 [bit_copy and bit_copy_n](#bit_copy-and-bit_copy_n)

<h2 id="Introduction">1 Introduction</h2>


Bitstrings store resizable data and provide access at a bit level.  Convenient ways to convert
bitstrings to and from strings and integers are provided.

Bit iterators are used to simplify the API and make it consistent with the C++ standard library.

Input and output bit streams can be used for writing and reading bits to and from a stream.

For API calls that use strings to encode a bitstring, the following convention is used.  Strings preceded with a `#`
denotes hexadecimal, an `@` symbol denotes binary.  Otherwise, strings are interpreted as hexadecimal.  Spaces are
ignored.

The following table provides examples on how to represent a bitstring in ASCII:

ASCII         | meaning
--------------|--------------
`@1110`       | binary 1110
`#1100`       | hexadecimal 1100
`1100`        | hexadecimal 1100
`FF`          | hexadecimal FF
`@FF`         | error! binary numbers must be 1 or 0
`#F`          | error! hexadecimal numbers must be byte aligned
`@1 101 01`   | binary 110101
`AA BB CC DD` | hexadecimal AABBCCDD

Examples of creating a bitstring:

```c++
auto a = ict::bitstring("FF");
auto b = ict::bitstring("#FF"); // same as a
auto c = ict::bitstring("@111");
```


<h2 id="bit_iterator">2 bit_iterator</h2>


Bit iterators are bidirectional.  They are simply a wrapper of a byte pointer and a bit offset.  `bitstring` provides
`bit_begin()` and `bit_end()` operations that behave as expected.

You can also create a `bit_iterator` outside of a `bitstring`, e.g:

    char x = 0xF0;
    auto i = ict::bit_iterator(&x, 5);

This iterator, for example, can then be used in other operations that use bit iterators, such as `copy` or `copy_n`
described below.

to get to the actual bit value a bit iterator points to:
    
    bool v = i->value();    i->value(true);     
    
<h2 id="ict::bitstring">3 ict::bitstring</h2>


Bitstrings are value types.

<h2 id="Constructors">3.1 Constructors</h2>


```c++
bitstring();                        // empty bitstring
bitstring(size_t bit_size);         // bitstring filled with 0 bits
bitstring(const bitstring & a);     // copy constructor
bitstring(bitstring && a) noexcept; // move constructor

// bitstring from input iterators.  These can can be bit_iterators described above, or traditional iterators.
bitstring(InputIterator first, InputIterator last);

// bitstring from an input iterator and bit length.
bitstring(InputIterator first, size_t bit_len);

// create from strings
bitstring(const char * str);
bitstring(const std::string & str);
```

<h2 id="Methods">3.2 Methods</h2>


```c++
// return a substring
bitstring substr(size_t index, size_t len = std::numeric_limits<size_t>::max()) const;
inline bitstring& remove(size_t index, size_t len); // remove a substring
void resize(size_t s);  // resize

bool empty() const // check for empty

pointer begin() const // return an iterator (this is a char *)
pointer end() const
bit_iterator bit_begin() const // return an iterator (this is a char *)
bit_iterator bit_end() const
pointer data() const // same as begin()

size_t byte_size() const // size in bytes
size_t bit_size() const  // size in bits

bool local() const // denotes if the bitstring is stored locally (true if its smaller than 64 bits)

void set(size_t index) // set a bit to 1
void reset(size_t index) // set a bit to 0
bool at(size_t index) const // get the bit value 

void clear()
```

<h2 id="ict::ibitstream">4 ict::ibitstream</h2>


An input bit stream is modeled after the std::istream.  However it acts upon bits and not bytes.  It provides
a convenient way to read bits from a bitstring (a protocol message, for example).

Example usage:

```c++
auto bits = ict::bitstring("@111000");
ict::ibitstream is(bits);
auto a = is.read(3); // a = @111
auto b = is.read(3); // b = @000
```


The only way to create an `ibitstream` is to initialize its constructor with a `bitstring`.  

```c++
ibitstream() = delete;
ibitstream(const ibitstream &) = delete;
ibitstream(const bitstring & bits)

// Read n bits.  Values of n greater than remaining() result in undefined behavior.
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
```


<h2 id="Constraints-and-Marks">4.1 Constraints and Marks</h2>


A constraint is use to temporarily restrict the length of the bitstring.  A mark is used to mark the current index in
the bitstring.  Calling `last_mark()` will return the index of the last mark set.

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

<h2 id="ict::obitstream">5 ict::obitstream</h2>


Output bit streams can be used to construct bitstrings from others.

Example:

```c++
ict::obitstream os;
os << ict::bitstring("@111");
os << ict::bitstring("@000");
auto bits = os.bits(); // bits = @111000
```


```c++
struct obitstream {
    // create a stream and initialize it with bits
    obitstream(const bitstring & bits)

    obitstream() // create obitstream
    obitstream& operator<<(const bitstring & b) // stream operator

    bitstring bits() // return contents of stream as a bitstring
};
```

<h2 id="Functions">6 Functions</h2>
<h2 id="reverse_bytes">6.1 reverse_bytes</h2>


```c++
inline void reverse_bytes(T & number)

```


Reverse the bytes of the bitstring (not the bits).

<h2 id="to_integer">6.2 to_integer</h2>

```c++
template <typename T>
inline T to_integer(bitstring const & bits, bool swap = true)
```

Convert a bitstring to an integer of a given type.  Little-endian representation is assumed.

<h2 id="from_integer">6.3 from_integer</h2>

```c++
template <typename T> 
inline bitstring from_integer(T number, size_t dest_size=sizeof(T) * 8)
```


Convert a number to a bitstring

<h2 id="gsm7">6.4 gsm7</h2>

```c++
inline std::string gsm7(const bitstring & bits, size_t fill_bits = 0)
```

Text messaging support, of course.

<h2 id="to_string">6.5 to_string</h2>

```c++
inline std::string to_string(const bitstring & bits);
```

Convert to std::string.  Byte aligned bitstrings will be returned in hex, otherwise binary.

<h2 id="operator<<">6.6 operator<<</h2>

```c++
inline std::ostream& operator<<(std::ostream& os, const bitstring & bits)
```

Output stream operator, uses to_string() above.

<h2 id="set_bit">6.7 set_bit</h2>

```c++
inline void set_bit(unsigned char * buf, unsigned index, bool val);
```

Set a bit.

<h2 id="bit">6.8 bit</h2>

```c++
inline bool bit(unsigned char * buf, unsigned index);
```

Get a bit.

<h2 id="bit_copy-and-bit_copy_n">6.9 bit_copy and bit_copy_n</h2>

```c++
inline void bit_copy(bit_iterator first, bit_iterator last, bit_iterator result)
inline void bit_copy_n(bit_iterator & first, size_t bit_count, bit_iterator & result) 
```

Eventually, you have to write code that actually does something.  This is it.  Copy a range of bits from one address
and bit offset to another.

You can use bit_copy by itself without having to create a bitstring.  For example, here is a function that takes
5 parameters and simply calls `bit_copy_n`:

    void my_copy(char * src, size_t src_bit_offset, size_t bit_len, char * res, size_t res_bit_offset) {
        ict::bit_copy_n({src, src_bit_offset}, bit_len, {res, res_bit_offset});
    }
