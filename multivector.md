# Multivector

```c++
#include <ict/multivector.h>
namespace ict 
```

A multivector is a generic container that behaves just like a `std::vector` except its iterators also
behave just like `std::vector`.  And since `std::vector` is used in the underlying representation, we can make
heirarchies that benefit from both cache friendly locality of reference and C++11 move semantics.

The current implementation includes only a subset of the features found std::vector and described below. 

A simple mutlivector can be created with:

```c++
    auto m = ict::multivector<int>{1, 2, {10, 11, 12, {100}}, 3}
    // print it out using convenience function:

    std::cout << ict::to_text(m);


    // and the output:
    1
    2
      10
      11
      12
        100
    3
```

In the above example, the values 1, 2, and 3 make a *sibling vector*,  and are considered the top level children.  
10, 11, and 12 make up a another sibling vector and 2 is their *parent*. The parent of 1 is considered the *root*.

`iterators` for multivectors are called `cursors` and are the means of navigating a multivector.  They are random access
(among siblings). Unlike regular iterators they have the vector functions you would expect: `begin()`, `end()`,
`emplace()`, etc.

For cursors, `begin()` returns a cursor to the first child, and `end()` returns a cursor after the last child.
So for multivectors, there can be many different `begin()` and `end()` cursors.  All cursors that are part 
of the same multivector are comparable.

## <a name="cursor"/> ict::multivector<T>::cursor 

In addition to normal random-access iterator operations, cursors provide the following functions:


Here are vector operations that are currently supported for cursors:

```c++
    cursor_type begin() 
    cursor_type begin() const 
    cursor_type cbegin() const 
    cursor_type end() 
    cursor_type end() const 
    cursor_type cend() const 

    root_cursor_type rbegin() const 

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

Additional operations provided for cursors:

```c++
    cursor_base leaf() // returns the last child or itself if it is empty()

    // return a child cursor 
    cursor_base operator[](difference_type i) const 

    bool first_item() const // return true if this is the first child (rename to first_child???)
    cursor_base parent() const  // return a cursor to parent
    bool is_root() const // true if this is the root cursor
}
```

## <a name="root_cursor"/> multivector<T>::root_cursor

A root cursor is a forward cursor.  `operator++` just goes up and to the left until the root.

For example:

```c++
    auto m = ict::multivector<int>{1, {2, { 10, 11, 12}, 3, 4, 5}};
    auto last = m.root().leaf(); // points to 5
    while (!last.is_root()) std::cout << *last << '\n';
```
will print out
```
    5
    4
    3
    2
    1
```

`root_cursor` supports the following vector operations:

```c++
    // vector operations
    bool empty() const
    size_t size() const
    cursor begin()
    cursor begin() const
    cursor cbegin() const
```

Additional operations in addition to forward iterators:
```c++
    bool is_root() const
```

template <typename ValueType, bool is_const_cursor>    
struct linear_cursor_base : public std::iterator<std::forward_iterator_tag, ValueType> {
    typedef linear_cursor_base linear_type;
    typedef linear_type & linear_reference;
    typedef linear_type * linear_pointer;
    typedef const linear_type & const_linear_reference;

    typedef cursor_base<ValueType, is_const_cursor> cursor_type;
    typedef typename cursor_type::pointer pointer;
    typedef typename cursor_type::reference reference;

    linear_cursor_base() {}
    linear_cursor_base(const linear_cursor_base<ValueType, false> & b) : c(b.c) {}
    linear_cursor_base(const cursor_type b) : c(b) {}

    reference operator*() const { return *c; }
    pointer operator->() const { return &(*c); }

    bool operator==(const_linear_reference b) const { return c == b.c; }

    bool operator!=(const_linear_reference b) const { return !operator==(b); }

    linear_reference operator++() {
        if (!c.empty()) {
            parents.push_back(c);
            c = c.begin();
        } else increment();
        return *this;
    }
    
    linear_type operator++(int) {
        auto temp = *this;
        operator++();
        return temp;
    }

    void increment() {
        if (!parents.empty() && (c == (parents.back().end() - 1))) {
            c = parents.back();
            parents.pop_back();
            increment(); // try again
        } else ++c;
    }

    //private:
    cursor_type c;
    std::vector<cursor_type> parents;
};

template <typename ValueType>
struct item {
    typedef ValueType value_type;
    typedef const value_type const_value_type;
    typedef value_type * pointer;
    typedef value_type & reference;
    typedef const value_type & const_reference;

    typedef std::vector<item> vector_type;
    typedef vector_type * vector_pointer;
    typedef const vector_type * const_vector_pointer;

    typedef typename std::vector<item>::iterator item_iterator;
    typedef typename std::vector<item>::const_iterator const_item_iterator;
    typedef item & item_reference;
    typedef const item & const_item_reference;
    typedef item * item_pointer;
    typedef const item * const_item_pointer;

    typedef size_t size_type;
    typedef int difference_type;

    //! Default constructor
    item() : parent{(item *)(-1)} { }

    //! Copy constructor
    item(const item & b) : parent(0) {
        parent = b.parent;
        value = b.value;
        nodes_ = b.nodes_;
        if (!nodes_.empty()) nodes_[0].parent = this;
    }

    item(item && b) NOEXCEPT : parent(0) {
        parent = b.parent;
        value = std::move(b.value);
        nodes_ = std::move(b.nodes_);
        if (!nodes_.empty()) nodes_[0].parent = this;
    }

    item(item * parent, const value_type & value) : parent(parent), value(value) { }

    template <class... Args>
    item(item * parent, Args&&... args) : parent(parent), value(std::forward<Args>(args)...) { }

    // copy assignable: a = b
    item& operator=(const item & b) {
        value = b.value;
        nodes_ = b.nodes_;
        if (!nodes_.empty()) nodes_[0].parent = this;
        return *this;
    }

    item& operator=(item && b) NOEXCEPT {
        value = std::move(b.value);
        nodes_ = std::move(b.nodes_);
        if (!nodes_.empty()) nodes_[0].parent = this;
        return *this;
    }

    item& operator=(value_type b) {
        value = b;
        return *this;
    }

    // equality
    friend bool operator==(const item & a, const item & b) {
        return (a.value == b.value) && (a.nodes_ == b.nodes_);
    }
       
    // not equality
    friend bool operator!=(const item & a, const item & b) {
        return !(a == b);
    }

    // less than 
    friend bool operator<(const  item & a, const item & b) {
        if (a.value < b.value) return true;
        else return a.nodes_ < b.nodes_;
    }

    //! emptiness
    bool empty() const { return nodes_.empty(); }

    //! clear
    void clear() { nodes_.clear(); }


    //! pop_back
    void pop_back() { nodes_.pop_back(); }

    //! size
    size_t size() const { return nodes_.size(); }

    size_t item_count() const {
        if (empty()) return 0;
        return item_count(&(nodes_[0]), &nodes_.back() + 1);
    }

    item_reference operator[](int index) { return nodes_[index]; }
    const_item_reference operator[](int index) const { return nodes_[index]; }

    template <class... Args>
    void emplace_back(Args&&... args) {
        nodes_.emplace_back(nullptr, std::forward<Args>(args)...);
        nodes_[0].parent = this;
    }

    template <class... Args>
    item_pointer emplace(Args&&... args) {
        emplace_back(std::forward<Args>(args)...);
        return &nodes_.back();
    }


    operator reference () { return value; }
    operator const_reference () const { return value; }

    bool is_root() const { return parent == (item *)(-1); }

    item_pointer parent_item() const {
        auto i = this;
        while (!i->parent) --i;
        return i->parent;
    }

    // promote the children of the last item 
    void promote_last() {
        // detach the last child
        if (nodes_.empty()) return;
        auto last = std::move(nodes_.back());
        last[0].parent = 0; // set its parent pointer to 0
        nodes_.resize(nodes_.size() - 1);

        if (last.empty()) return;
        // now move its contents 
        std::move(last.nodes_.begin(), last.nodes_.end(), std::back_inserter(nodes_));

        nodes_[0].parent = this; // in case nodes_ was reallocated

    }

    const_vector_pointer vec_pointer() const { return &nodes_; }
    vector_pointer vec_pointer() { return &nodes_; }

    item_pointer begin_ptr() { return &(nodes_[0]); }
    item_pointer end_ptr() { return &nodes_.back() + 1; }
    const_item_pointer begin_ptr() const { return &(nodes_[0]); }
    const_item_pointer end_ptr() const { return &nodes_.back() + 1; }
    const_item_pointer cbegin_ptr() const { return begin_ptr(); }
    const_item_pointer cend_ptr() const { return end_ptr(); }


    item_pointer parent;
    value_type value;
    vector_type nodes_;

    private:
    template <typename T>
    size_t item_count(T first, T last) const {
        size_t n = 0;
        for (auto i = first; i != last; ++i) {
            if (!i->nodes_.empty()) n += item_count(i->nodes_.begin(), i->nodes_.end());
            ++n;
        }
        return n;
    }

};

// initialization list helper type
template <typename T>
struct init_list_type {
    init_list_type(T v) : d(v) {}
    init_list_type(std::initializer_list<init_list_type<T>> l) : l(l) {}

    template <typename Cursor>
    void add(Cursor parent) const {
        if (is_list()) {
            if (l.begin()->is_list()) {
                // create a default constructed T and add it if "{{" occurs in initializer list
                parent.leaf().emplace_back();
            }
            for ( const auto & e: l) e.add(--parent.end());
        } else {
            parent.emplace_back(d);
        }
    }

    bool is_list() const { return l.size() > 0; }

private:
    std::initializer_list<init_list_type<T>> l;
    T d;
};

template <typename value_type>
struct multivector {
    typedef bool is_multivector;
    typedef item<value_type> item_type;
    typedef item<value_type> * item_pointer;
    typedef item<value_type> & item_reference;
    typedef const item<value_type> * const_item_pointer;
    typedef const item<value_type> & const_item_reference;

    typedef cursor_base<value_type, false> cursor;
    typedef cursor_base<value_type, true> const_cursor;
    typedef root_cursor_base<value_type, false> root_cursor;
    typedef root_cursor_base<value_type, true> const_root_cursor;
    typedef linear_cursor_base<value_type, false> linear_cursor;
    typedef linear_cursor_base<value_type, true> const_linear_cursor;

    // Semiregular
    // default constructable: multivector a;
    multivector() {
        root_.value = value_type();
        root_.parent = (item<value_type> *)(-1);
    } 

    // copy constructable: multivector a = b;
    multivector(const multivector & b) : root_(b.root_) { };

    multivector(multivector && b) NOEXCEPT {
        root_ = std::move(b.root_);
    };

    // Conversions
    multivector(cursor a) : root_(a.item_ref()) {
        root_.value = value_type(); // weird
        root_.parent = (item<value_type> *)(-1);
    }

    // initialization list
    multivector(std::initializer_list<init_list_type<value_type>> l) {
        root_.value = value_type();
        root_.parent = (item<value_type> *)(-1);
        for (const auto & e : l) e.add(root());
    }

    multivector(std::initializer_list<init_list_type<const char *>> l) {
        root_.value = value_type();
        root_.parent = (item<value_type> *)(-1);
        for (const auto & e : l) e.add(root());
    }

    // assignment
    multivector& operator=(const multivector& b) {
        root_ = b.root_;
        return *this;
    }

    multivector& operator=(multivector&& b) NOEXCEPT {
        root_ = std::move(b.root_);
        return *this;
    }

    // Regular
    // equality
    friend bool operator==(const multivector & a, const multivector & b) {
        return a.root_ == b.root_;
    }

    // not equal
    friend bool operator!=(const multivector & a, const multivector & b) {
        return !(a == b);
    }

    // TotallyOrdered
    friend bool operator<(const multivector & a, const multivector & b) {
        return a.root_ < b.root_;
    }

    friend bool operator>(const multivector& a, const multivector& b) {
        return b < a;
    }
    friend bool operator<=(const multivector& a, const multivector& b) {
        return !(b < a);
    }
    friend bool operator>=(const multivector& a, const multivector& b) {
        return !(a < b);
    } 

    item_reference operator[](int index) { return root()[index]; }
    const_item_reference operator[](int index) const { return root()[index]; }

    //! clear
    void clear() { root_.clear(); }
    void pop_back() { root().pop_back(); }

    bool empty() const { return root_.empty(); }
    cursor root() { return cursor(nullptr, &root_); }
    const_cursor root() const { return const_cursor(nullptr, &root_); }
    root_cursor rend() { return typename item<value_type>::root_cursor(&root_); }

    size_t size() const { return root_.item_count(); }
    cursor begin() { return root().begin(); }
    const_cursor begin() const { return root().begin(); }
    const_cursor cbegin() const { return root().begin(); }

    cursor end() { return root().end(); }
    const_cursor end() const { return root().end(); }
    const_cursor cend() const { return root().end(); }

    item<value_type> root_;
};

template <typename T>
inline std::ostringstream & operator<<(std::ostringstream & ss, item<T> & a) {
    ss << a.value  << " (" <<  &a << ", " << a.parent << ")";
    return ss;
}

template <typename T>
inline std::ostringstream & operator<<(std::ostringstream & ss, typename item<T>::cursor & a) {
    ss << a->value << " (" <<  &(*a) << ", " << a->parent << ")";
    return ss;
}

// multivector algorithms

// return the root cursor of a multivector given a cursor
template <typename Cursor>
Cursor get_root(Cursor start) {
    auto r = typename Cursor::root_cursor_type(start);
    while (!r.is_root()) ++r;
    return r;
}

// return the previous cursor, either a sibling or parent
template <typename Cursor>
Cursor previous(Cursor self) {
    auto r = typename Cursor::root_cursor_type(self);
    ++r;
    return r;
}


// recursively descend and perform an action on each item
template <typename Cursor, typename Action>
void recurse(Cursor parent, Action action) {
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


// verify the internal integrity of the multivector
template <typename T>
void verify(T parent) {
    recurse(parent, [](T self, T) {
        size_t count = 0;
        if (self.empty() && self.size() != 0) IT_PANIC("empty cursor has non-zero size");
        if (!self.empty()) {
            if (self[0].it_->parent == 0) IT_PANIC("parent set to 0");
            if (self[0].it_->parent != &(self.item_ref())) {
                IT_PANIC("incorrect first child " << self[0].it_->parent << ", " << &(*self));
            }

            ++count;
            for (auto i = self.begin() + 1; i != self.end(); ++i, ++count) {
                if (i.item_ref().parent != 0) IT_PANIC("non-first child of self is not zero");
            }

            if (count != self.size()) IT_PANIC("incorrect size");
        }
        get_root(self); // make sure this doesn't seg fault
    });
}

template <typename T>
inline void verify(const multivector<T> & tree) {
    if (tree.root().item_ref().parent != (item<T> *)(-1)) {
        IT_PANIC("root parent is not valid: " << tree.root().item_ref().parent);
    }
    verify(tree.root());
}

// convert to a compact string 
template <typename T> 
inline std::string compact_string(T parent) {
    std::ostringstream ss;
    recurse(parent, 
        [&](T self, T parent, int) {
            ss << *self;
            if (!self.empty()) ss << " {";
            else if (self != --parent.end()) ss << ' ';
        }, 

        [&](T self, T, int) {
            if (!self.empty()) ss << "} ";
    });

    // convert all "} }" patterns to "}}"
    auto x = ss.str();
    ict::replace(x, " }", "}");
    ict::normalize(x);
    return x;
}

template <typename T>
inline std::string compact_string(const multivector<T> & tree) {
    return compact_string(tree.root());
}

// convert to a table string
template <typename T>
inline std::string cursor_to_text(T parent) {
    std::ostringstream ss;
    recurse(parent, 
        [&](T self, T, int level) {
            ss << ict::spaces(level * 2) << *self << '\n';
        },

        [&](T, T, int) { } // nothing to do on the way up
    );
    return ss.str();
}

template <typename T>
std::string to_text(const multivector<T> & tree) {
    return ict::cursor_to_text(tree.root());
}

template <typename T>
std::string to_debug_text(const multivector<T> & tree) {
    typedef typename multivector<T>::const_cursor cursor_type;
    std::ostringstream ss;
    auto r = tree.root();
    ss << *r << " " << r.item_ref().parent << '\n';
    recurse(r, 
        [&](cursor_type self, cursor_type, int level) {
            ss << ict::spaces(level * 2) << *self << " ";
            ss << self.it_->parent << '\n';
        },

        [&](cursor_type, cursor_type, int) { } // nothing to do on the way up
    );
    return ss.str();
}


// find and rfind algorithms
struct path {
    typedef std::vector<std::string>::iterator iterator;
    typedef std::vector<std::string>::const_iterator const_iterator;

    path(const std::vector<std::string> & path, bool abs = true) : p(path), abs(abs) { }

    path(const std::string & path_string) {
        auto ps = path_string;
        if (ps.size() == 0) IT_PANIC("invalid empty path");
        abs = true;
        if (ps[0] == '/') { 
            abs = true;
            ps.erase(0, 1);
            if (ps.size() == 0) IT_PANIC("a lone '/' is an invalid path");
            if (ps[0] == '/') { 
                abs = false;
                ps.erase(0, 1);
                if (ps.size() == 0) IT_PANIC("a lone '//' is an invalid path");
            }
        }
        p = ict::escape_split(ps, '/');
    }

    path(const char * path_string) : path(std::string(path_string)) {}

    iterator begin() { return p.begin(); }
    const_iterator begin() const { return p.begin(); }
    const_iterator cbegin() const { return p.begin(); }
    iterator end() { return p.end(); }
    const_iterator end() const { return p.end(); }
    const_iterator cend() const { return p.end(); }
    bool absolute() const { return abs; }
    void absolute(bool abs) { this->abs = abs; }
    bool empty() const { return p.empty(); }
    size_t size() const { return p.size(); }
    private:
    std::vector<std::string> p;
    bool abs = false;
};

inline std::ostream & operator<<(std::ostream & os, const path & p) {
    if (p.absolute()) os << "/";
    ict::join(os, p.cbegin(), p.cend(), "/");
    return os;
}

template <typename T>  
inline bool leaf_test(const T &) { return 1; }

template <typename T> 
inline std::string name_of(const T & a) { return a.name; }

template <>
inline std::string name_of(const std::string & value) { return value; }

template <>
inline std::string name_of(const int & value) { return ict::to_string(value); }

namespace util {
    template <typename Cursor, typename Op, typename Test>
    inline Cursor rfind_x(Cursor first, const std::string & name, Op op, Test test) {
        typedef typename Cursor::root_cursor_type root_cursor;
        auto c = root_cursor(first);
        while (!c.is_root()) {
            if (op(*c) == name && test(*c)) return c;
            ++c;
        }
        return c;
    }
    template <typename Cursor, typename PathIter, typename Op, typename Test>
    inline Cursor find_x(Cursor parent, PathIter first, PathIter last, Op op, Test test) {
        if (parent.empty()) return parent.end();
        for (auto i = parent.begin(); i!=parent.end(); ++i) {
            if (op(*i) == *first) {
                if (first + 1 == last) {
                    if (test(*i)) return i;
                } else {
                    auto n = find_x(i, first + 1, last, op, test);
                    if (n != i.end()) return n;
                }
            }
        }
        return parent.end();
    }
}

// find given a path
template <typename Cursor, typename Op, typename Test>
inline Cursor find(Cursor parent, const path & path, Op op, Test test) {
    typedef typename Cursor::linear_type iterator;
    if (path.absolute()) return util::find_x(parent, path.begin(), path.end(), op, test);
    else {
        for (iterator i = parent.begin(); i!= parent.end(); ++i) {
            if (op(*i) == *path.begin()) {
                if (path.begin() + 1 == path.end()) {
                    if (test(*i)) return i;
                } else {
                    auto c = Cursor(i);
                    auto x = util::find_x(c, path.begin() + 1, path.end(), op, test);
                    if (x != c.end()) return x;
                }
            }
        }
        return parent.end();
    }
}

template <typename Cursor, typename Op>
inline Cursor find(Cursor parent, const path & path, Op op) {
    typedef typename Cursor::value_type value_type;
    return find(parent, path, op, [](const value_type &){ return true; });
}

template <typename Cursor>
inline Cursor find(Cursor parent, const path & path) {
    return find(parent, path, name_of<typename Cursor::value_type>);
}

template <typename Cursor, typename Op, typename Test>
inline Cursor rfind(Cursor first, const path & path, Op op, Test test) {
    typedef typename Cursor::root_cursor_type root_cursor;
    if (!path.absolute()) IT_PANIC("path must be absolute for rfind()");
    if (path.size() == 1) return util::rfind_x(first, *path.begin(), op, test);
    
    auto rfirst = root_cursor(first);

    while (!rfirst.is_root()) {
        if (op(*rfirst) == *path.begin()) {
            auto parent = Cursor(rfirst);
            auto x = util::find_x(parent, path.begin() + 1, path.end(), op, test);
            if (x != parent.end()) return x;
        }
        ++rfirst;
    }
    return rfirst;
}

// rfind given a path
template <typename Cursor, typename Op>
inline Cursor rfind(Cursor first, const path & path, Op op) {
    typedef typename Cursor::value_type value_type;
    return rfind(first, path, op, [](const value_type &){ return true; });
}

template <typename Cursor>
inline Cursor rfind(Cursor first, const path & path) {
    return rfind(first, path, name_of<typename Cursor::value_type>);
}

template <typename S, typename C> 
inline void path_string(S & ss, C c) {
    if (!c.is_root()) {
        path_string(ss, c.parent());
        ss << '/';
        ss << name_of(*c);
    }
}

// return the path of a cursor
template <typename T, typename C = typename T::is_cursor> 
inline std::string path_string(T c) {
    std::ostringstream ss;
    path_string(ss, c);
    return ss.str();
}

template <typename Cursor> 
inline void promote_last(Cursor parent) {
    parent.promote_last();
}

template <typename T>
inline std::ostream & operator<<(std::ostream & ss, const multivector<T> & a) {
    ss << to_text(a);
    return ss;
}
}
