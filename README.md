# Intrig C++ Tools

**ict** is a small set of openly available reusable components originally developed and used by Intrig for its
next-generation message decoding engine.

The goal of **ict** is to provide useful independent components with a natural, simple interface enabling you to
concentrate on your task at hand, not wrestle with the tools you are using. The only dependency is the standard template
library.

The header files contain the complete implementation so you will not need to compile any source files.  **ict** is
released under the permissive MIT license.

Initially, two components are available, with more to come: `ict::bitstring` and `ict::multivector`.  Below is a brief
description and links to their respective documentation.  

Everything here is a work in progress.  There are definitely gaps in the features provided.  We tend to only implement
the functionality we need.  Hopefully you will find ways to contribute the things you need also.

The intended use is for you to add ict as a submodule to your projects include directory.  From time to time, you can
then update when new features are added.  However, strict backward compatibility is not a priority for ict development
and the release version will indicate if anything has changed.


## ict::bitstring

A `bitstring` provides a way to access arbitrarily sized data at a bit level.  Bitstrings smaller than 64
bits require no memory allocation.  Functions are provided to convert bitstrings to and from ASCII hex or binary strings
and integers.  Corresponding input and output streams for bits are provided with `ict::ibitstream` and
`ict::obitstream`.  This enables you to easily dissect or catenate bitstrings without being concerned with byte
alignment.

**[ict::bitstring tutorial and reference](docs/bitstring.md)**

## ict::multivector

A `multivector` is a generic container that behaves just like a `std::vector` except its iterators also
behave just like `std::vector`.  And since `std::vector` is used in the underlying representation, we can make
heirarchies that benefit from both cache friendly locality of reference and C++11 move semantics.

**[ict::multivector tutorial and reference](docs/multivector.md)**

## Building unit tests

Although using the tools does not require any source files, running the unit tests does.  Just type `make` at the top
level to invoke the cmake based build system.  And `make test` will run the unit tests.
