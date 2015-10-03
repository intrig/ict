# Intrig C++ Tools

*ict* is a small set of openly available reusable components developed and used by Intrig.  They are a result of
countless hours of programming missteps, restarts, and refactoring.

The aim is to provide a natural, simple interface with minimal dependencies (usually just the stl)
and enable you to concentrate on the task at hand, not the tools you are using.

Initially, two components are available: `ict::bitstring` and `ict::multivector`.

A bitstring contains an arbitrary number of bytes.  Bitstrings smaller than 64 bits require no memory allocation.
Corresponding input and output streams for bits are provided with `ict::ibitstream` and `ict::obitstream`.

A multivector is a generic container that behaves just like a std::vector, except its iterators also behave just like
std::vectors.  Since std::vector is used in the underlying representation, we can make heirarchies that benefit from
both cache friendly locality of reference and C++11 move semantics.


