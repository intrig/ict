#include "multivectorunit.h"

#include <bitstring.h>
#include <multivector.h>

void multivector_unit::empty_multivectors() {
    // default constructor
    ict::multivector<int> a;
    IT_ASSERT(a.empty());
    IT_ASSERT(a.size() == 0);
    IT_ASSERT(a.root().empty());

    // copy constructable
    ict::multivector<int> b(a);
    IT_ASSERT(b.empty());
    IT_ASSERT(b.size() == 0);
    IT_ASSERT(b.root().empty());

    // assignable
    ict::multivector<int> c;
    c = a;

    IT_ASSERT(c.empty());

    // equality
    IT_ASSERT(a == a);
    IT_ASSERT(a == b);
    IT_ASSERT(a == c);
    IT_ASSERT(b == c);

    // clear
    a.clear();
    IT_ASSERT(a.empty());
    IT_ASSERT(a.size() == 0);
    IT_ASSERT(a.root().empty());
}

void multivector_unit::adding() {
    typedef ict::multivector<int> imultivector;
    {
        ict::multivector<int> tree;
        tree.root().emplace(42);
        IT_ASSERT(!tree.root().empty());
        IT_ASSERT(tree.size() == 1);
        IT_ASSERT(tree.root().size() == 1);

        imultivector::cursor root = tree.root();
        imultivector::cursor first = root.begin();
        IT_ASSERT(*first == 42);

        IT_ASSERT(*tree.root().begin() == 42);
        IT_ASSERT(tree.root().begin().empty());
    }
    {
        ict::multivector<int> tree;
        tree.root().emplace(43).emplace(44).emplace(45);
        IT_ASSERT(tree.root().size() == 1);
        IT_ASSERT(tree.root().begin().size() == 1);
        IT_ASSERT(tree.root().begin().begin().size() == 1);
        IT_ASSERT(tree.size() == 3);

        // clear and repeat
        tree.clear();
        tree.root().emplace(43).emplace(44).emplace(45);
        IT_ASSERT(tree.root().size() == 1);
        IT_ASSERT(tree.root().begin().size() == 1);
        IT_ASSERT(tree.root().begin().begin().size() == 1);
        IT_ASSERT(tree.size() == 3);
    }
}

static void size_one_test(const ict::multivector<int> &x) {
    IT_ASSERT(!x.empty());
    IT_ASSERT(x.size() == 1);
    IT_ASSERT(!x.root().empty());
    IT_ASSERT(x.root().size() == 1);
    IT_ASSERT(x.root()[0] == 42);
    IT_ASSERT(*x.root().begin() == 42);
    IT_ASSERT(x.root().begin().empty());
}

static void size_one_mutate(ict::multivector<int> &x) {
    auto i = x.root().begin();
    IT_ASSERT_MSG(*i, *i == 42);
    *i = 21;
    IT_ASSERT_MSG(*i, *i == 21);
    IT_ASSERT(*x.root().begin() == 21);
    *i = 42;
    IT_ASSERT(*x.root().begin() == 42);
}

static void size_one_copy_mutate(ict::multivector<int> x) { size_one_mutate(x); }

void multivector_unit::size_one() {

    // default constructor
    ict::multivector<int> a;
    a.root().emplace(42);
    size_one_test(a);

    // copy constructable
    ict::multivector<int> b(a);
    size_one_test(b);

    // assignable
    ict::multivector<int> c;
    c = a;
    size_one_test(c);

    // equality
    IT_ASSERT(a == a);
    IT_ASSERT(a == b);
    IT_ASSERT(a == c);
    IT_ASSERT(b == c);

    // copy and mutate
    size_one_copy_mutate(a);
    size_one_test(a); // make sure it didn't change

    // mutate
    size_one_mutate(a);
}

static void size_two_test(std::string m, ict::multivector<int> &x) {
    IT_ASSERT_MSG(m, !x.empty());
    IT_ASSERT_MSG(m, x.size() == 2);
    IT_ASSERT_MSG(m, !x.root().empty());
    IT_ASSERT_MSG(m, x.root().size() == 1);
    IT_ASSERT_MSG(m, x.root().begin().size() == 1);
    IT_ASSERT_MSG(m, x.root()[0] == 42);
    IT_ASSERT_MSG(m, x.root().begin()[0] == 43);
    IT_ASSERT_MSG(m, x.root().begin()[0] == 43);
    IT_ASSERT_MSG(m, !x.root().begin().empty());
    IT_ASSERT_MSG(m, x.root().begin().begin().empty());
    IT_ASSERT_MSG(m, x.root().begin().begin().empty());
}

void multivector_unit::size_two() {
    // default constructor
    ict::multivector<int> a;
    a.root().emplace(42).emplace(43);
    IT_ASSERT(a == a);
    size_two_test("default constructor", a);

    // copy constructable
    ict::multivector<int> b(a);
    size_two_test("copy constructor", b);

    // assignable
    ict::multivector<int> c;
    c = a;
    size_two_test("copy constructor", c);

    // equality
    IT_ASSERT(a == b);
    IT_ASSERT(a == c);
    IT_ASSERT(b == c);
}

template <typename T> void add_recursive(T x, unsigned int n) {
    auto y = x.emplace(n);
    if (n > 0)
        add_recursive(y, n - 1);
}

static void create_complicated(ict::multivector<int> &tree) {
    add_recursive(tree.root(), 3);
    add_recursive(tree.root(), 3);
    add_recursive(tree.root(), 3);

    tree.root().emplace(33).emplace(34).emplace(35);

    int c = 4;
    for (auto n : {1, 2, 3}) {
        tree.root().emplace(n);
        auto i = tree.root().end() - 1;
        for (int x = 0; x < 3; ++x) {
            i.emplace(c++);
            auto j = i.begin() + i.size() - 1;
            for (int y = 0; y < 2; ++y)
                j.emplace(y);
        }
    }
}

static ict::multivector<int> create_complicated() {
    ict::multivector<int> comp;
    create_complicated(comp);
    return comp;
}

void multivector_unit::complicated() {

    ict::multivector<int> a = create_complicated();

    IT_ASSERT(a.size() == 45);

    // IT_ASSERT(a.root().child_count() == 45);

    // copy constructable
    ict::multivector<int> b(a);

    IT_ASSERT(!a.empty());
    IT_ASSERT(!b.empty());
    // assignable
    ict::multivector<int> c;
    c = a;

    IT_ASSERT(!c.empty());

    // equality
    IT_ASSERT(a == b);
    IT_ASSERT(a == c);
    IT_ASSERT(b == c);

    auto x = ict::compact_string(c);
    IT_ASSERT_MSG(x, x == "3 {2 {1 {0}}} 3 {2 {1 {0}}} 3 {2 {1 {0}}} 33 {34 "
                          "{35}} 1 {4 {0 1} 5 {0 1} 6 {0 1}} 2 {7 {0 1} 8 {0 "
                          "1} 9 {0 1}} 3 {10 {0 1} 11 {0 1} 12 {0 1}}");

    c.root()[0] = 1;

    x = ict::compact_string(c);
    IT_ASSERT_MSG(x, x == "1 {2 {1 {0}}} 3 {2 {1 {0}}} 3 {2 {1 {0}}} 33 {34 "
                          "{35}} 1 {4 {0 1} 5 {0 1} 6 {0 1}} 2 {7 {0 1} 8 {0 "
                          "1} 9 {0 1}} 3 {10 {0 1} 11 {0 1} 12 {0 1}}");
}

void multivector_unit::iterators() {
    ict::multivector<int> tree;
    tree.root().emplace(1);
    tree.root().emplace(2).emplace(10).emplace(100);
    tree.root().emplace(3).emplace(11).emplace(101);
    ict::verify(tree);

    auto s = ict::to_text(tree);

    for (auto i = tree.begin(); i != tree.end(); ++i) {
        ict::multivector<int>::cursor c = i;
        ict::verify(c);
    }
}

struct boo {
    boo() : n(0) {}
    ~boo() {}
    boo(int n, const ict::bitstring &bits, const std::string &name)
        : n(n), bits(bits), name(name) {}
    boo(const boo &b) = default;
    boo &operator=(const boo &) = default;

    int n;
    ict::bitstring bits;
    std::string name;
    bool operator==(const boo &b) const {
        return n == b.n && bits == b.bits && name == b.name;
    }
};

void multivector_unit::non_pod() {
    ict::multivector<boo> tree;
    tree.root().emplace_back(boo{0, "@1", "mark"});
    tree.root().emplace_back(boo{1, "@11", "allan"});
    tree.root().emplace_back(boo{2, "@111", "beckwith"});
    tree.root().emplace_back(boo{3, "@1111", "phil"});
    tree.clear();
    tree.root().emplace(0, "@1", "mark");
    tree.root().emplace(1, "@11", "allan");
    tree.root().emplace(2, "@111", "beckwith");
    tree.root().emplace(3, "@1111", "phil");

    auto i = tree.root().begin();
    IT_ASSERT(i->n == 0);
    IT_ASSERT(i->bits == "@1");
    IT_ASSERT(i->name == "mark");
    IT_ASSERT(*i == (boo{0, "@1", "mark"}));

    auto j = i;
    IT_ASSERT(j->n == 0);
    IT_ASSERT(j->bits == "@1");
    IT_ASSERT(j->name == "mark");
    IT_ASSERT(*j == (boo{0, "@1", "mark"}));

    auto b = boo{4, "@0", "joe"};
    *j = b;
    IT_ASSERT(j->n == 4);
    IT_ASSERT(j->bits == "@0");
    IT_ASSERT(j->name == "joe");
    IT_ASSERT(*j == (boo{4, "@0", "joe"}));

    IT_ASSERT(i->n == 4);
    IT_ASSERT(i->bits == "@0");
    IT_ASSERT(i->name == "joe");
    IT_ASSERT(*i == (boo{4, "@0", "joe"}));
}

struct Custom {
    Custom() {}
    Custom(const std::string &name, int length, uint64_t value,
           const std::string &description)
        : name(name), length(length), value(value), description(description) {}
    std::string name;
    int length;
    uint64_t value;
    std::string description;
    bool operator==(const std::string &n) const { return name == n; }
    bool operator==(const std::string &n) { return name == n; }
};

void multivector_unit::initializer_list() {
    auto tree = ict::multivector<int>{1, 2, {4, 5, 6, {7, 8, 9}}, 5, {7}, 3};
    IT_ASSERT(ict::compact_string(tree) == "1 2 {4 5 6 {7 8 9}} 5 {7} 3");

    auto tree2 = ict::multivector<int>{1, 2, {{4, 5, 6, {7, 8, 9}}}, 5, {7}, 3};
    IT_ASSERT(ict::compact_string(tree2) == "1 2 {0 {4 5 6 {7 8 9}}} 5 {7} 3");
}

void multivector_unit::totally_ordered() {
    auto a = ict::multivector<std::string>{"a"};
    auto b = ict::multivector<std::string>{"b"};
    IT_ASSERT(a < b);
    auto c = ict::multivector<std::string>{"a", "b"};
    IT_ASSERT(a < c);
    IT_ASSERT(b > c);

    auto a1 = ict::multivector<std::string>{"a", {"b"}};
    IT_ASSERT(a < a1);
    auto a2 = ict::multivector<std::string>{"a", {"c"}};
    IT_ASSERT(a1 < a2);
    IT_ASSERT(a2 > a1);
    IT_ASSERT(a2 != a1);
    auto a3 =
        ict::multivector<std::string>{"a", {"b", "c", "d", {"e", "f", "g"}}};
    auto a4 =
        ict::multivector<std::string>{"a", {"b", "c", "d", {"e", "f", "g"}}};
    IT_ASSERT(a3 == a4);
}

void multivector_unit::moving() {
    auto a3 =
        ict::multivector<std::string>{"a", {"b", "c", "d", {"e", "f", "g"}}};
}

template <typename T>
void compare_linear(const ict::multivector<T> &a, const std::vector<T> b) {
    auto first = ict::to_linear(a.begin());
    auto last = ict::to_linear(a.end());
    IT_ASSERT(std::equal(first, last, b.begin()));
}

void multivector_unit::linear() {
    auto a1 = ict::multivector<std::string>{"a", "b", "c"};
    auto v1 = std::vector<std::string>{"a", "b", "c"};
    compare_linear(a1, v1);

    a1 = ict::multivector<std::string>{"a", {"b"}, "c"};
    compare_linear(a1, v1);

    a1 = ict::multivector<std::string>{"a", "b", {"c"}};
    compare_linear(a1, v1);

    a1 = ict::multivector<std::string>{"a", {"b", "c"}};
    compare_linear(a1, v1);

    a1 = ict::multivector<std::string>{"a", {"b", {"c"}}};
    compare_linear(a1, v1);
}

void multivector_unit::promote() {
    {
        auto a = ict::multivector<int>{1, 2, 3, {10, 11, 12}};
        auto b = ict::multivector<int>{1, 2, 10, 11, 12};

        IT_ASSERT(a != b);

        ict::promote_last(a.root());
        ict::verify(a);

        IT_ASSERT(a == b);
    }
    {
        auto a = ict::multivector<int>{1, 2, 3, {10, 11, 12, {100, 101, 102}}};
        auto b = ict::multivector<int>{1, 2, 10, 11, 12, {100, 101, 102}};

        IT_ASSERT(a != b);
        a.root().promote_last();
        ict::verify(a);

        IT_ASSERT_MSG(ict::to_text(a), a == b);
    }
    {
        auto a = ict::multivector<int>{
            1, 2, 3, {10, 11, 12, {100, 101, 102, {201, 202, 203}}}};
        auto b = ict::multivector<int>{
            1, 2, 10, 11, 12, {100, 101, 102, {201, 202, 203}}};

        IT_ASSERT(a != b);
        a.root().promote_last();
        ict::verify(a);

        IT_ASSERT_MSG(ict::to_text(a), a == b);
    }
}

void multivector_unit::ascending() {
    auto m = ict::multivector<int>{1, {10, {100, 101, 102}}, 2, 3, 4};
    auto last = ict::to_ascending(--m.end());
    IT_ASSERT_MSG(*last, *last == 4);
    std::ostringstream os;
    while (!last.is_root()) {
        os << *last << ' ';
        ++last;
    }
    auto str = os.str();
    IT_ASSERT_MSG(str, str == "4 3 2 1 ");
}

void multivector_unit::ascending2() {
    std::ostringstream os;
    auto m = ict::multivector<int>{1, {10, {100, 101, 102}}, 2, 3, 4};

    auto n = m.root().begin().begin().end(); // n points to one past 102
    --n;                                     // n points to 102
    IT_ASSERT_MSG(*n, *n == 102);
    auto last = ict::multivector<int>::ascending_cursor(
        n); // convert to an ascending cursor
    // last points to 102
    IT_ASSERT_MSG(*last, *last == 102);
    auto x = ict::to_ascending(--m.begin().begin().end());
    IT_ASSERT(*x == 102);
    IT_ASSERT(last == x);
    while (!last.is_root()) {
        os << *last << ' ';
        ++last;
    }
    IT_ASSERT_MSG(os.str(), os.str() == "102 101 100 10 1 ");
}

void multivector_unit::append_children() {
    auto m = ict::multivector<int>{1, {10, {100, 101, 102}}, 2, 3, 4};
    auto q = ict::multivector<int>();
    ict::append(q.root(), m.root());
    IT_ASSERT(q == m);

    auto c = q.begin().begin();
    auto r = ict::multivector<int>{99, 100, 101};
    IT_ASSERT(*c == 10); // make sure we are where we think we are
    ict::append(c, r.root());
    auto s =
        ict::multivector<int>{1, {10, {100, 101, 102, 99, 100, 101}}, 2, 3, 4};
    IT_ASSERT(q == s);
}

// Make sure our cursors behave well with the stl.
void multivector_unit::stl_algo() {
    {
        // basic
        auto m = ict::multivector<int>{1, {10, {100, 101, 102}}, 2, 3, 4};
        auto i = std::find(m.begin(), m.end(), 3);
        IT_ASSERT(*i == 3);
        i = std::find(m.begin(), m.end(), -3);
        IT_ASSERT(i == m.end());
    }
    {
        // ascending
        auto m = ict::multivector<int>{1, {10, {100, 101, 102}}, 2, 3, 4};

        auto first = ict::to_ascending(--m.begin().begin().end());
        IT_ASSERT(*first == 102);

        auto last = ict::to_ascending(m.root());
        IT_ASSERT(last.is_root());

        auto i = std::find(first, last, 10);
        IT_ASSERT(*i == 10);

        i = std::find(first, last, -42);
        IT_ASSERT(i == last);

        i = std::find(first, ict::to_ascending(m.root()), -23);
        IT_ASSERT(i == ict::to_ascending(m.root()));
    }
        // linear
    {
    }
}


int main(int, char **) {
    multivector_unit test;
    ict::unit_test<multivector_unit> ut(&test);
    return ut.run();
}
