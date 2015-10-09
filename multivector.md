# Multivector

```c++
#include <ict/multivector.h>
namespace ict 
```

A multivector is a generic container that behaves just like a `std::vector` except its iterators also
behave just like `std::vector`.  And since `std::vector` is used in the underlying representation, we can make
heirarchies that benefit from both cache friendly locality of reference and C++11 move semantics.

* [multivector&lt;T&gt;::cursor](#cursor)
* [multivector&lt;T&gt;::ascending_cursor](#ascending_cursor)
* [multivector&lt;T&gt;::linear_cursor](#linear_cursor)
* [multivector&lt;T&gt;](#multivector_struct)
* [Find Algorithm](#find)
* [Functions](#functions)
* [References](#references)

A simple mutlivector can be created and displayed with:

```c++
    auto m = ict::multivector<int>{1, 2, {10, 11, 12, {100}}, 3}
    // print it out using convenience function:

    std::cout << ict::to_text(m);
```
and the output:

```c++
    1
    2
      10
      11
      12
        100
    3
```

In the above example, the values 1, 2, and 3 make a *sibling vector*, and are considered the top level *children*.  1
precedes 2 and 2 precedes 3.  10, 11, and 12 make up a another sibling vector and 2 is their *parent*. The parent of 1
is considered the *root*.

`iterators` for multivectors are called `cursors` and are the means of navigation.  They are random access
(among siblings). Unlike regular iterators they have the vector functions you would expect: `begin()`, `end()`,
`emplace()`, etc.

For cursors, `begin()` returns a cursor to the first child, and `end()` returns a cursor after the last child.
So for multivectors, there can be many different `begin()` and `end()` cursors.  All cursors that are part 
of the same multivector are comparable.

The current implementation includes only a subset of the features found `std::vector` and described below. 

## <a name="cursor"/> multivector&lt;T&gt;::cursor 

In addition to normal random-access iterator operations, cursors provide the following functions:


Here are vector operations that are currently supported for cursors:

```c++
    cursor_type begin() 
    cursor_type begin() const 
    cursor_type cbegin() const 
    cursor_type end() 
    cursor_type end() const 
    cursor_type cend() const 
    
    bool empty() const 
    size_t size() const 

    void reserve(size_t n)

    void clear()
    void pop_back()

    template <class... Args>
    cursor_base emplace(Args&&... args) 

    template <class... Args>
    void emplace_back(Args&&... args) 
```

Additional cursor operations:

```c++
    // return a child cursor 
    cursor_base operator[](difference_type i) const 

    bool is_first_child() const // return true if this is the first child 
    cursor_base parent() const  // return a cursor to parent
    bool is_root() const // true if this is the root cursor
}
```

Cursor validity is similar to that of vectors.  If a sibling vector gets resized, then all its cursors are invalidated.
But any parent cursor is still valid.

## <a name="ascending_cursor"/> multivector&lt;T&gt;::ascending_cursor

An ascending cursor is a forward cursor.  `operator++` just goes up and to the left until the root.

For example:

```c++
    auto m = ict::multivector<int>{1, {10, { 100, 101, 102}}, 2, 3, 4};
    auto last = ict::ascending_begin(m.root()[0][0]);
    // last points to 102
    while (!last.is_root()) {
        std::cout << *last << '\n';
        ++last;
    }
```
will print out
```c++
    102
    101
    100
    10
    1
```

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

## <a name="linear_cursor"/> multivector&lt;T&gt;::linear_cursor

A linear cursor is also a forward iterator.  It traverses a multivector in a depth-first order.

The following code:

```c++
    auto m = ict::multivector<int>{1, { 2, { 3 }}};
    for (auto i = ict::linear_begin(m.root());  i!=m.end(); ++i) std::cout << *i << '\n';
```
will output:
```
    1
    2
    3
```

Notice the automatic conversion from one type of cursor to another.

There are no operations for the linear cursor other than those of an input iterator.

## <a name="multivector_struct"/> multivector&lt;T&gt;

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
    // c will become the root of a new multivector, its contents will be copied.
    multivector(cursor c)
```

Additional operations supported:

```c++
    // return the parent of the first item.
    cursor root()
    const_cursor root() const 
```

## <a name="find"/> Find Algorithm

The `find` algorithm presented here for multivectors will find an item in a multivector based on a path.  The path
is specified with a string.

```c++
inline Cursor find(Cursor parent, const path & path) 

// find using an ascending cursor.
inline Cursor rfind(Cursor first, const path & path)
```

`ict::path` is a struct that takes a string in its constructor.

For the following example:
```c++
        auto tree = ict::multivector<std::string> 
        { "one", 
          "two", 
          { "four", 
            "five", 
            { "one", 
              "two", 
              { "six", 
                "seven" 
              }, 
              "three" 
            }
          }, 
          "three" 
        };
```

The following holds true:

find call                                         |     return result
--------------------------------------------------|-------------
`auto c = ict::find(tree.root(), "two")`          | `*c == "two"`
`auto c = ict::find(tree.root(), "two/four")`     | `*c == "four"`
`auto c = ict::find(tree.root(), "two/four")`     | `*c == "four"`
`auto c = ict::find(tree.root(), "two/six")`      | `c == tree.end()`
`auto c = ict::find(tree.root(), "two/four/one")` | `c == tree.end()`
`auto c = ict::find(tree.root(), "two/five/one")` | `*c == "one"`
`auto c = ict::find(tree.root(), "two/five/two")` | `*c == "two"`

The paths above are relative to a parent cursor.  A search can be done that finds a path anywhere among the 
descendents by preceding the path with `//`;

`find` uses the following template function to determine what to use for the path:

```c+++
template <typename T> 
inline std::string name_of(const T & a) { return a.name; }
```

You can specialize the `name_of` function for your custom types.

## <a name="functions"/>Functions

```c++
// return the root cursor of a multivector given a cursor
Cursor get_root(Cursor start)

// return the previous cursor, either a sibling or parent
Cursor previous(Cursor self)

// recursively descend and perform an action on each item
template <typename Cursor, typename Action>
void recurse(Cursor parent, Action action) 
    for (auto i = parent.begin(); i != parent.end(); ++i) {
        action(i, parent);
        recurse(i, action);
    }
}

// recursively descend and perform an action on each item the way down and up
template <typename Cursor, typename ActionDown, typename ActionUp>
void recurse(Cursor parent, ActionDown action_down, ActionUp action_up, int level = 0) {
    for (auto i = parent.begin(); i != parent.end(); ++i) {
        action_down(i, parent, level);
        recurse(i, action_down, action_up, level + 1);
        action_up(i, parent, level);
    }
}

// convert to a compact string, similar to an initializer list.
inline std::string compact_string(Cursor parent)
inline std::string compact_string(const multivector<T> & tree)

// convert to a table string
inline std::string cursor_to_text(Cursor parent)
std::string to_text(const multivector<T> & tree) 

// return the path of a cursor
inline std::string path_string(Cursor c)

// returns the last child of c or c if it is empty()
inline Cursor leaf(Cursor c) 

// Replace the last child with the children of the last child.  
// This should be rewritten to not be so specific.
inline void promote_last(Cursor parent)

// return an ascending cursor.  It points to the last child of parent.
inline typename Cursor::ascending_cursor_type ascending_begin(Cursor parent)

// return a linear cursor to the first child
inline typename Cursor::linear_type linear_begin(Cursor parent)

// return a linear cursor to one past the last child
inline typename Cursor::linear_type linear_end(Cursor parent)

// print multivector using the to_text() function
template <typename T>
inline std::ostream & operator<<(std::ostream & ss, const multivector<T> & a)

```

## <a name="references"/>References

Below are other tree implementations and papers I looked at while developing the multivector.  In general, they provide
more capability than the multivector, but are node based.

`multivector` has some commonalty with the boost property tree:
http://www.boost.org/doc/libs/1_59_0/doc/html/property_tree.html

Adobe forest:
http://stlab.adobe.com/classadobe_1_1forest.html

tree.hh:
http://tree.phi-sci.com/documentation.html

Hierarchical Data Structures and Related Concepts for the C++ Standard Library:
http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3700.html

