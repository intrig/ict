# ict
Intrig C++ Tools

*ict* is a small set of openly available reusable components developed and used by Intrig.  They are a result of
countless hours of programming, restarts, and refactoring.  We are left with an almost trivially simple set
of components that will hopefully prompt you to think, "That doesn't seem very hard."  

Initially, two components are available: `ict::bitstring` and `ict::multivector`.

A bitstring contains an arbitrary number of bytes.  Bitstrings smaller than 64 bits require no memory allocation.
Corresponding input and output streams for bits are provided with `ict::ibitstream` and `ict::obitstream`.

A multivector is a generic container that behaves just like a std::vector, except its iterators also behave just like
std::vectors.  Since std::vector is used in the underlying representation, we can make heirarchies that benefit from
cache friendly locality of reference.  Also `multivector` would not be feasible without C++11 move semantics.

