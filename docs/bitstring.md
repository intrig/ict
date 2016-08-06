#  Bit strings and streams
```c++
#include <ict/bitstring.h>
namespace ict
```

* 1 [Introduction ](#1)
* 2 [ict::bitstring ](#2)
    * 2.1 [Constructors ](#2.1)
    * 2.2 [Operators ](#2.2)
    * 2.3 [Methods ](#2.3)
* 3 [ict::ibitstream ](#3)
    * 3.1 [Constraints and Marks ](#3.1)
* 4 [ict::obitstream ](#4)
* 5 [Functions ](#5)
    * 5.1 [reverse_bytes ](#5.1)
    * 5.2 [to_integer ](#5.2)
    * 5.3 [from_integer ](#5.3)
    * 5.4 [gsm7 ](#5.4)
    * 5.5 [to_string ](#5.5)
    * 5.6 [operator<< ](#5.6)
    * 5.7 [set_bit ](#5.7)
    * 5.8 [bit ](#5.8)
    * 5.9 [bit_copy ](#5.9)

##<a name="1"/>1 Introduction 


Bitstrings store resizable data and provide access at a bit level.  There are convenient ways of converting 
bitstrings to and from strings and integers.

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

##<a name="2"/>2 ict::bitstring 
###<a name="2.1"/>2.1 Constructors 


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
###<a name="2.2"/>2.2 Operators 

```c++
bitstring & operator=(const bitstring & b);
bitstring & operator=(bitstring && b) noexcept;
friend bool operator==(const bitstring & a, const bitstring & b);
friend bool operator!=(const bitstring & a, const bitstring & b);
```
###<a name="2.3"/>2.3 Methods 


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
```
##<a name="3"/>3 ict::ibitstream 


An input bit stream is modeled after the std::istream.  However it acts upon bits and not bytes.  It provides
a convenient way to read bits from a bitstring (a protocol message, for example).

Example usage:

```c++
auto bits = ict::bitstring("@111000");
ict::ibitstream is(bits);
auto a = is.read(3); // a = @111
auto b = is.read(3); // b = @000
```

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
```

###<a name="3.1"/>3.1 Constraints and Marks 


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
##<a name="4"/>4 ict::obitstream 


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
##<a name="5"/>5 Functions 
###<a name="5.1"/>5.1 reverse_bytes 


```c++
inline void reverse_bytes(T & number)

```

Reverse the bytes of the bitstring (not the bits).

###<a name="5.2"/>5.2 to_integer 

```c++
template <typename T>
inline T to_integer(bitstring const & bits, bool swap = true)
```
Convert a bitstring to an integer of a given type.  Little-endian representation is assumed.

###<a name="5.3"/>5.3 from_integer 

```c++
template <typename T> 
inline bitstring from_integer(T number, size_t dest_size=sizeof(T) * 8)
```

Convert a number to a bitstring

###<a name="5.4"/>5.4 gsm7 

```c++
inline std::string gsm7(const bitstring & bits, size_t fill_bits = 0)
```
Text messaging support, of course.

###<a name="5.5"/>5.5 to_string 

```c++
inline std::string to_string(const bitstring & bits);
```
Convert to std::string.  Byte aligned bitstrings will be returned in hex, otherwise binary.

###<a name="5.6"/>5.6 operator<< 

```c++
inline std::ostream& operator<<(std::ostream& os, const bitstring & bits)
```
Output stream operator, uses to_string() above.

###<a name="5.7"/>5.7 set_bit 

```c++
inline void set_bit(unsigned char * buf, unsigned index, bool val);
```
Set a bit.

###<a name="5.8"/>5.8 bit 

```c++
inline bool bit(unsigned char * buf, unsigned index);
```
Get a bit.

###<a name="5.9"/>5.9 bit_copy 

```c++
inline void bit_copy(char * dest, size_t desto, const char * src, size_t srco, size_t bit_length);
```
Eventually, you have to write code that actually does something.  This is it.  Copy a range of bits from one address
and bit offset to another.  The API should be improved by using array_view.  
See [issue #1](https://github.com/intrig/ict/issues/1).

