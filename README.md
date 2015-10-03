# Intrig C++ Tools

*ict* is a small set of openly available reusable components developed and used by Intrig.  They are a result of
countless hours of programming missteps, restarts, and refactoring.

The aim is to provide a natural, simple interface with minimal dependencies (usually just the stl)
and enable you to concentrate on your task at hand, not wrestle with the tools you are using.

Initially, two components are available: `ict::bitstring` and `ict::multivector`.  Below is a brief description
and links to their respective documentation.  

And one more thing, everything here is a work in progress.  Nothing, especially multivector, is a complete
implementation yet.  We tend to only implement the functionality we need.  Hopefully you will find ways to 
contribute the things you need also.

## ict::bitstring

A [bitstring](bitstring.md) contains an arbitrary number of bits.  Bitstrings smaller than 64 bits require no memory
allocation.  Functions are provided to convert bitstrings to and from ASCII hex or binary strings and integers.
Corresponding input and output streams for bits are provided with `ict::ibitstream` and `ict::obitstream`.  This enables
you to easily disect or catenate bitstrings without being concerned with byte alignment.

## ict::multivector

A [multivector](multivector.md) is a generic container that behaves just like a `std::vector` except its iterators also
behave just like `std::vector`s.  And since `std::vector`s are used in the underlying representation, we can make
heirarchies that benefit from both cache friendly locality of reference and C++11 move semantics.

The header files contain the complete implementation so you will not need to compile any source files.

## Building unit tests and examples

Just type `make` at the top level to invoke the cmake based build system.  And `make test` will run the unit tests.


