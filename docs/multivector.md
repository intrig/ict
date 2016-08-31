#  Multivector Tutorial and Reference
```c++
#include <ict/multivector.h>
```
*namespace ict*

* 1 [Introduction ](#1)
    * 1.1 [Example ](#1.1)
    * 1.2 [Download ](#1.2)
* 2 [Cursors ](#2)
    * 2.1 [multivector<T>::cursor ](#2.1)
    * 2.2 [multivector<T>::ascending_cursor ](#2.2)
    * 2.3 [multivector<T>::linear_cursor ](#2.3)
* 3 [Functions ](#3)
    * 3.1 [get_root ](#3.1)
    * 3.2 [previous ](#3.2)
    * 3.3 [recurse ](#3.3)
    * 3.4 [recurse (2) ](#3.4)
    * 3.5 [compact_string ](#3.5)
    * 3.6 [to_text ](#3.6)
    * 3.7 [leaf ](#3.7)
    * 3.8 [promote_last ](#3.8)
    * 3.9 [to_ascending ](#3.9)
    * 3.10 [to_linear ](#3.10)
    * 3.11 [append ](#3.11)
* 4 [References ](#4)

##<a name="1"/>1 Introduction 


A multivector to provide a hierarchical data structure with the convenience of a `std::vector`.

It is a generic container that behaves just like a `std::vector` except its iterators also
behave just like `std::vector`.  And since `std::vector` is used in the underlying representation, we can make
hierarchies that benefit from both cache friendly locality of reference and C++11 move semantics.

A multivector is optimised to handle trees with nodes that are likely to have siblings and less likely to have children.
This is exactly the kind of trees we find throughout computer programming: HTML, XML, user interface window hierarchies,
telecom messages, configurators, multiple choice tests, *more examples*, etc. 

###<a name="1.1"/>1.1 Example 


A simple multivector of integers can be created with:

```c++
    auto m = ict::multivector<int>{1, 2, {10, 11, 12, {100}}, 3};
```

and displayed with using a convenience function:

```c++
    std::cout << ict::to_text(m);
```

and the output:

```
    1
    2
      10
      11
      12
        100
    3
```

In the above example, the values 1, 2, and 3 make a *sub-vector*, and are considered the top level *children*.  1
precedes 2 and 2 precedes 3.  10, 11, and 12 make up a another sub-vector and 2 is their *parent*. The parent of 1
is considered the *root*.

`iterators` for multivectors are called `cursors` and are the means of navigation.  They are random access
(among siblings). Unlike regular iterators they have the vector functions you would expect: `begin()`, `end()`,
`emplace()`, etc.

For cursors, `begin()` returns a cursor to the first child, and `end()` returns a cursor after the last child.
So for multivectors, there can be many different `begin()` and `end()` cursors.  All cursors that are part 
of the same multivector are comparable.

The implementation currently includes a **subset** of the features found `std::vector` and is described
below. 


The multivector is a totally ordered container class for hierarchies.  It supports the following vector operations:

```c++
item_reference operator[](int index) 
const_item_reference operator[](int index) const 

void clear() 
void pop_back() 

bool empty() const 

size_t size() const 
cursor begin() 
const_cursor begin() const 
const_cursor cbegin() const 

cursor end() 
const_cursor end() const 
const_cursor cend() const 
```

There is an additional constructor that takes a cursor:

```c++
multivector(cursor c)
```

`c` will become the root of a new multivector, its contents will be copied.

###<a name="1.2"/>1.2 Download 


`multivector.h` is part of the Intrig C++ Toolkit and can be found at <https://github.com/intrig/ict>.

##<a name="2"/>2 Cursors 


Cursors share both the properties of `std::vector` and a std::vector's iterator.  This in effect makes a cursor
behave like a multivector.  There are three kinds of cursors, described in the following sections.

###<a name="2.1"/>2.1 multivector<T>::cursor 


In addition to normal random-access iterator operations, cursors provide the following functions:

Here are vector operations that are currently supported for cursors:

```c++
    cursor begin() 
    cursor begin() const 
    cursor cbegin() const 
    cursor end() 
    cursor end() const 
    cursor cend() const 
    
    bool empty() const 
    size_t size() const 

    void reserve(size_t n)

    void clear()
    void pop_back()

    // emplace a value at the end and return a cursor to it.
    template <class... Args>
    cursor emplace(Args&&... args) 

    // emplace a value at the end.
    template <class... Args>
    void emplace_back(Args&&... args) 
```

Additional cursor navigation operations are available:

```c++
// return a child cursor 
bool is_first_child() const // return true if this is the first child 
cursor parent() const // return a cursor to parent
bool is_root() const // true if this is the root cursor
```

Cursor validity is similar to that of vectors.  If a sibling vector gets resized, then all its cursors are invalidated.
But any parent cursor is still valid.

###<a name="2.2"/>2.2 multivector<T>::ascending_cursor 


An ascending cursor is a forward cursor.  `operator++` just goes up and to the left until the root.

For example:

```c++
auto m = ict::multivector<int>{1, {10, { 100, 101, 102}}, 2, 3, 4};
auto n = m.begin().begin().end();  // n points to one past 102
--n;                               // n points to 102
auto last = ict::multivector<int>::ascending_cursor(n); // convert to an ascending cursor

while (!last.is_root()) {
    std::cout << *last << '\n';
    ++last;
}
```

will print out

```
102
101
100
10
1
```

The above code segment that assigns the `last` cursor could be written more concisely using the `to_ascending()`
function:

    auto last = ict::to_ascending(--m.begin().begin().end());

`ascending_cursor` supports the following vector operations:

```c++
// vector operations
bool empty() const
size_t size() const
cursor begin()
cursor begin() const
cursor cbegin() const
```

Additional ascending cursor operations:

```c++
bool is_root() const
```
###<a name="2.3"/>2.3 multivector<T>::linear_cursor 


A linear cursor is also a forward iterator.  It traverses a multivector in a depth-first order.

The following code:

```c++
auto m = ict::multivector<int>{1, { 2, { 3 }, 4}};
for (auto i = ict::to_linear(m.begin());  i!=m.end(); ++i) std::cout << *i << '\n';
```

will output:

```
1
2
3
4
```

Notice the automatic conversion from one type of cursor to another.

There are no operations for the linear cursor other than those of an input iterator.

##<a name="3"/>3 Functions 


The multivector functions act upon one or more template cursor parameters that must satisfy the cursor 
definition above.

###<a name="3.1"/>3.1 get_root 


```c++
template <typename Cursor>
Cursor get_root(Cursor start)
```

Return the root cursor of a multivector given a cursor.  This is a log2(n) operation.
###<a name="3.2"/>3.2 previous 

```c++
template <typename Cursor>
Cursor previous(Cursor self)
```

Return the previous cursor, either a sibling or parent.
###<a name="3.3"/>3.3 recurse 

```c++
template <typename Cursor, typename Action>
void recurse(Cursor parent, Action action) 
```

Recursively descend and perform an action on each item.  The action must have a signature of:

`void action(Cursor current, Cursor parent)`;

`current` is the current item visited, and `parent` is its parent.

The following example will print out all the items in a multivector:

```c++
typedef ict::multivector<int>::cursor int_cursor;
auto m = ict::multivector<int>{1, { 2, { 3 }}};
ict::recurse(m.root(), [](int_cursor c, int_cursor) { std::cout << *c << '\n'; }
```
###<a name="3.4"/>3.4 recurse (2) 


```c++
template <typename Cursor, typename Action>
void recurse(Cursor parent, Action action_down, Action action_up, int level = 0) 
```

This version of recurse is similar to the above, except it also performs and action on the way up.
Also, the current depth in the tree will be provided.

###<a name="3.5"/>3.5 compact_string 

```c++
inline std::string compact_string(Cursor parent);
inline std::string compact_string(const multivector<T> & tree);
```

Conveniently return a compact string representation of a multivector.  It uses the above recurse method.

```c++
auto m = ict::multivector<int>{1, { 2, { 3 }}};
std::cout << ict::compact_string(m.root());
```

prints:

`{1 {2 {3}}}`
###<a name="3.6"/>3.6 to_text 

```c++
inline std::string to_text(Cursor parent)
inline std::string to_text(const multivector<T> & tree) 
```

Convert to a table string.  An example is provided in the introduction.

###<a name="3.7"/>3.7 leaf 


`inline Cursor leaf(Cursor c)`

Returns the last child of c or c if it is empty().
###<a name="3.8"/>3.8 promote_last 


`inline void promote_last(Cursor parent)`

Replace the last child with the children of the last child.  This should be rewritten to not be so specific.  There
should be a detach() ability that removes a subtree as a multivector.

###<a name="3.9"/>3.9 to_ascending 

`inline typename Cursor::ascending_cursor_type to_ascending(Cursor parent)`

Convert a cursor to an ascending cursor.
###<a name="3.10"/>3.10 to_linear 

`inline typename Cursor::linear_type to_linear(Cursor parent)`

Convert a cursor to a linear cursor.
###<a name="3.11"/>3.11 append 

```c++
template <typename Cursor, typename ConstCursor>
void append(Cursor parent, ConstCursor first, ConstCursor last) 

template <typename Cursor, typename ConstCursor>
void append(Cursor parent, ConstCursor from_parent)
```

Append (i.e., copy) the children of one cursor to the children of another.  The the children will be
appended to any existing children.
##<a name="4"/>4 References 


Below are other tree implementations and papers I looked at while developing multivector.  In general, they provide
more capability than the multivector, but are node based.

* multivector has some commonalty with the boost property tree:
  [boost property tree](http://www.boost.org/doc/libs/1_59_0/doc/html/property_tree.html)

* [Adobe forest](http://stlab.adobe.com/classadobe_1_1forest.html)

* [tree.hh](http://tree.phi-sci.com/documentation.html)

* [Hierarchical Data Structures and Related Concepts for the C++ Standard Library](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3700.html)

* [LCRS Binary Tree](https://en.wikipedia.org/wiki/Left-child_right-sibling_binary_tree)
