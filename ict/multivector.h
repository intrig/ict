#pragma once
//-- Copyright 2016 Intrig
//-- see https://github.com/intrig/xenon for license

#include <ict/ict.h>
#include <sstream>
#include <exception>
#include <vector>
#include <string>
#include <type_traits>
#include <functional>

namespace ict {

// forward declare item
template <typename ValueType> 
struct item;

// forward declare linear_cursor_base
template <typename ValueType, bool is_const_iterator>    
struct linear_cursor_base;

// forward declare ascending cursor
template <typename ValueType, bool is_const_cursor>    
struct ascending_cursor_base;

// Random Access (among siblings)
template <typename ValueType, bool is_const_cursor>
struct cursor_base : public std::iterator<std::bidirectional_iterator_tag, ValueType> {
    typedef bool is_cursor;
    typedef ValueType value_type;
    typedef item<ValueType> item_type;
    typedef std::vector<item_type> vec_type;

    typedef typename std::conditional<is_const_cursor, const ValueType *, ValueType *>::type pointer;
    typedef typename std::conditional<is_const_cursor, const ValueType &, ValueType &>::type reference;

    typedef typename std::conditional<is_const_cursor, const item_type *, item_type *>::type item_pointer;
    typedef typename std::conditional<is_const_cursor, const item_type &, item_type &>::type item_reference;

    typedef typename std::conditional<is_const_cursor, const vec_type *, vec_type *>::type vec_pointer;

    typedef cursor_base cursor_type;
    typedef cursor_type & cursor_reference;
    typedef cursor_type * cursor_pointer;
    typedef const cursor_type & const_cursor_reference;

    typedef ascending_cursor_base<ValueType, is_const_cursor> ascending_cursor_type;
    typedef ascending_cursor_type & ascending_cursor_reference;
    typedef ascending_cursor_type * ascending_cursor_pointer;
    typedef const ascending_cursor_type & const_ascending_cursor_reference;

    typedef linear_cursor_base<ValueType, is_const_cursor> linear_type;

    typedef int difference_type;

    // constructors
    cursor_base() : v(nullptr), it_(nullptr) {}
    cursor_base(ascending_cursor_reference b) : it_(b.it_) {
        v = &b.it_->parent_item()->nodes_;
    }
    cursor_base(const cursor_base<ValueType, false>& b) : v(b.v), it_(b.it_) {}
    cursor_base(vec_pointer v, item_pointer it) : v(v), it_{it} {}
    cursor_base(const linear_type b) : v(b.c.v), it_(b.c.it_) { }

    // cursor operations
    reference operator*() const { return it_->value; }
    pointer operator->() const { return &(it_->value); }

    item_reference item_ref() const { return *it_; }
    item_pointer item_ptr() const { return &(*it_); }

    cursor_reference operator++() {
        ++it_;
        return *this;
    }

    cursor_base operator++(int) {
        cursor_base temp=*this;
        ++*this;
        return temp;
    }

    cursor_reference operator--() {
        --it_;
        return *this;
    }

    cursor_base operator--(int) {
        cursor_base temp=*this;
        --*this;
        return temp;
    }

    bool operator==(const_cursor_reference b) const { 
        return v == b.v && it_ == b.it_; 
    }
    bool operator!=(const_cursor_reference b) const { return !operator==(b); }

    // random access
    cursor_reference operator+=(difference_type i) {
        it_ += i;
        return *this;
    }

    cursor_reference operator-=(difference_type i) {
        *this += -i;
        return *this;
    }

    reference operator[](difference_type i) const { return *(begin() + i); }
    friend cursor_base operator-(cursor_base x, difference_type i) { return x + (-i); }
    friend difference_type operator-(cursor_base& x, cursor_base& y) { return x.it_ - y.it_; }

    friend cursor_base operator+(cursor_base x, difference_type i) { return x += i; }
    friend cursor_base operator+(difference_type i, cursor_base x) { return x += i; }

    friend bool operator<(cursor_base& x, cursor_base& y) { return x - y < 0; }
    friend bool operator>(cursor_base& x, cursor_base& y) { return y < x; }
    friend bool operator<=(cursor_base& x, cursor_base& y) { return !(y < x); }
    friend bool operator>=(cursor_base& x, cursor_base& y) { return !(x < y); }

    // cursor specific operations
    bool empty() const { return it_->empty(); }

    cursor_type begin() { return cursor_base(it_->vec_pointer(), it_->begin_ptr()); }
    cursor_type begin() const { return cursor_base(it_->vec_pointer(), it_->begin_ptr()); }
    cursor_type cbegin() const { return cursor_base(it_->vec_pointer(), it_->begin_ptr()); }
    cursor_type end() { return cursor_base(it_->vec_pointer(), it_->end_ptr()); }
    cursor_type end() const { return cursor_base(it_->vec_pointer(), it_->end_ptr()); }
    cursor_type cend() const { return cursor_base(it_->vec_pointer(), it_->end_ptr()); }

    size_t size() const { return it_->size(); }

    template <class... Args>
    cursor_base emplace(Args&&... args) {
        return cursor_base(it_->vec_pointer(), it_->emplace(std::forward<Args>(args)...));
    }

    void reserve(size_t n) { it_->nodes_.reserve(n); }

    void clear() { it_->nodes_.clear(); }
    void pop_back() { it_->pop_back(); }
    void promote_last() { it_->promote_last(); }

    template <class... Args>
    void emplace_back(Args&&... args) {
        it_->emplace_back(std::forward<Args>(args)...);
    }

    bool is_first_child() const { return it_->parent != nullptr; }

    cursor_base parent() const {
        return cursor_base(&it_->parent_item()->parent_item()->nodes_, it_->parent_item());
    }
    bool is_root() const { return it_->is_root(); }

    friend struct cursor_base<ValueType, false>;
    
    vec_pointer v;
    item_pointer it_;
};

// Forward iterator
// operator++ just goes up and to the left until the root.
template <typename ValueType, bool is_const_cursor>    
struct ascending_cursor_base : public std::iterator<std::forward_iterator_tag, ValueType> {
    typedef ValueType value_type;
    typedef item<ValueType> item_type;

    typedef typename std::conditional<is_const_cursor, const ValueType *, ValueType *>::type pointer;
    typedef typename std::conditional<is_const_cursor, const ValueType &, ValueType &>::type reference;

    typedef typename std::conditional<is_const_cursor, const item_type *, item_type *>::type item_pointer;
    typedef typename std::conditional<is_const_cursor, const item_type &, item_type &>::type item_reference;

    typedef cursor_base<ValueType, is_const_cursor> cursor_type;
    typedef cursor_type & cursor_reference;
    typedef cursor_type * cursor_pointer;
    typedef const cursor_type & const_cursor_reference;

    typedef ascending_cursor_base ascending_cursor_type;
    typedef ascending_cursor_type & ascending_cursor_reference;
    typedef ascending_cursor_type * ascending_cursor_pointer;
    typedef const ascending_cursor_type & const_ascending_cursor_reference;

    typedef int difference_type;

    // constructors
    ascending_cursor_base() {}
    ascending_cursor_base(cursor_reference b) : it_(b.it_) {}
    ascending_cursor_base(const ascending_cursor_base<ValueType, false>& b) : it_(b.it_) {}
    ascending_cursor_base(const item_pointer it) : it_{it} {}

    // cursor operations
    reference operator*() const { return it_->value; }
    pointer operator->() const { return &(it_->value); }

    item_reference item_ref() const { return *it_; }
    item_pointer item_ptr() const { return &(*it_); }

    ascending_cursor_reference operator++() {
        if (it_->parent == 0) --it_;
        else it_ = it_->parent;
        return *this;
    }

    ascending_cursor_type operator++(int) {
        ascending_cursor_base temp=*this;
        ++*this;
        return temp;
    }

    bool operator==(const_ascending_cursor_reference it) const { return it_ == it.it_; }

    bool operator!=(const_ascending_cursor_reference it) const { return !operator==(it); }

    // cursor specific operations
    bool empty() const { return it_->empty(); }
    size_t size() const { return it_->size(); }
    cursor_type begin() { return it_->begin(); }
    cursor_type begin() const { return it_->begin(); }
    cursor_type cbegin() const { return it_->begin(); }
    bool is_root() const { return it_->is_root(); }

    friend struct ascending_cursor_base<ValueType, false>;

    item_pointer it_;
};

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

    item(item && b) noexcept : parent(0) {
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

    item& operator=(item && b) noexcept {
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

    void insert_parent() {
        // detach all the children;
        auto t = nodes_;
        nodes_.clear();
        nodes_.emplace_back();
        nodes_[0].parent = this;
        nodes_[0].nodes_ = t;
        nodes_[0].nodes_[0].parent = &nodes_[0];
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
                leaf(parent).emplace_back();
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
    typedef ascending_cursor_base<value_type, false> ascending_cursor;
    typedef ascending_cursor_base<value_type, true> const_ascending_cursor;
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

    multivector(multivector && b) noexcept {
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

    multivector& operator=(multivector&& b) noexcept {
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

    template <class... Args>
    void emplace_back(Args&&... args) {
        root().emplace_back(std::forward<Args>(args)...);
    }

    //! clear
    void clear() { root_.clear(); }
    void pop_back() { root().pop_back(); }

    bool empty() const { return root_.empty(); }
    cursor root() { return cursor(nullptr, &root_); }
    const_cursor root() const { return const_cursor(nullptr, &root_); }

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
    auto r = typename Cursor::ascending_cursor_type(start);
    while (!r.is_root()) ++r;
    return r;
}

// return the previous cursor, either a sibling or parent
template <typename Cursor>
Cursor previous(Cursor self) {
    auto r = typename Cursor::ascending_cursor_type(self);
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
        if (self.empty() && self.size() != 0) std::runtime_error("empty cursor has non-zero size");
        if (!self.empty()) {
            if (self.begin().it_->parent == 0) std::runtime_error("parent set to 0");
            if (self.begin().it_->parent != &(self.item_ref())) {
                std::ostringstream os;
                os << "incorrect first child " << self.begin().it_->parent << ", " << &(*self);
                std::runtime_error(os.str());
            }

            ++count;
            for (auto i = self.begin() + 1; i != self.end(); ++i, ++count) {
                if (i.item_ref().parent != 0) std::runtime_error("non-first child of self is not zero");
            }

            if (count != self.size()) std::runtime_error("incorrect size");
        }
        get_root(self); // make sure this doesn't seg fault
    });
}

template <typename T>
inline void verify(const multivector<T> & tree) {
    if (tree.root().item_ref().parent != (item<T> *)(-1)) {
        std::ostringstream os;
        os << "root parent is not valid: " << tree.root().item_ref().parent;
        std::runtime_error(os.str());
    }
    verify(tree.root());
}

// convert to a compact string 
template <typename Cursor> 
inline std::string compact_string(Cursor parent) {
    std::ostringstream ss;
    recurse(parent, 
        [&](Cursor self, Cursor parent, int) {
            ss << *self;
            if (!self.empty()) ss << " {";
            else if (self != --parent.end()) ss << ' ';
        }, 

        [&](Cursor self, Cursor, int) {
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
template <typename Cursor>
inline std::string to_text(Cursor parent) {
    std::ostringstream ss;
    recurse(parent, 
        [&](Cursor self, Cursor, int level) {
            ss << ict::spaces(level * 2) << *self << '\n';
        },

        [&](Cursor, Cursor, int) { } // nothing to do on the way up
    );
    return ss.str();
}

template <typename T>
inline std::string to_text(const multivector<T> & tree) {
    return ict::to_text(tree.root());
}

template <typename T>
inline std::string to_debug_text(const multivector<T> & tree) {
    typedef typename multivector<T>::const_cursor cursor_type;
    std::ostringstream ss;
    auto r = tree.root();
    //to_debug_text(ss, *r);
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

template <typename Cursor>
inline Cursor leaf(Cursor c) {
    if (c.empty()) return c;
    return --c.end();
}

template <typename Cursor> 
inline void promote_last(Cursor parent) {
    parent.promote_last();
}

template <typename Cursor>
inline typename Cursor::ascending_cursor_type to_ascending(Cursor c) {
    return c;
}

template <typename Cursor> 
inline typename Cursor::linear_type to_linear(Cursor c) {
    return c;
}

// recursive copy all children
template <typename Cursor, typename ConstCursor>
void append(Cursor parent, ConstCursor first, ConstCursor last) {
    while (first != last) {
        auto c = parent.emplace(*first);
        append(c, first.begin(), first.end());
        ++first;
    }
}

template <typename Cursor, typename ConstCursor>
void append(Cursor parent, ConstCursor from_parent) {
    append(parent, from_parent.begin(), from_parent.end());
}

#if 0 // this may not be a good idea
template <typename T>
inline std::ostream & operator<<(std::ostream & ss, const multivector<T> & a) {
    ss << to_text(a);
    return ss;
}
#endif
}
