#include "multivectorunit.h"

#include <ict/multivector.h>
#include <ict/bitstring.h>

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

void size_one_test(const ict::multivector<int> & x) {
    IT_ASSERT(!x.empty());
    IT_ASSERT(x.size() == 1);
    IT_ASSERT(!x.root().empty());
    IT_ASSERT(x.root().size() == 1);
    IT_ASSERT(*x.root()[0] == 42);
    IT_ASSERT(*x.root().begin() == 42);
    IT_ASSERT(x.root().begin().empty());
}

void size_one_mutate(ict::multivector<int> & x) {
    auto i = x.root().begin();
    IT_ASSERT_MSG(*i, *i == 42);
    *i = 21;
    IT_ASSERT_MSG(*i, *i == 21);
    IT_ASSERT(*x.root().begin() == 21);
    *i = 42;
    IT_ASSERT(*x.root().begin() == 42);
}

void size_one_copy_mutate(ict::multivector<int> x) {
    size_one_mutate(x);
}

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

void size_two_test(std::string m, ict::multivector<int> & x) {
    IT_ASSERT_MSG(m, !x.empty());
    IT_ASSERT_MSG(m, x.size() == 2);
    IT_ASSERT_MSG(m, !x.root().empty());
    IT_ASSERT_MSG(m, x.root().size() == 1);
    IT_ASSERT_MSG(m, x.root().begin().size() == 1);
    IT_ASSERT_MSG(m, *x.root()[0] == 42);
    IT_ASSERT_MSG(m, *x.root().begin()[0] == 43);
    IT_ASSERT_MSG(m, *x.root()[0][0] == 43);
    IT_ASSERT_MSG(m, !x.root().begin().empty());
    IT_ASSERT_MSG(m, x.root().begin().begin().empty());
    IT_ASSERT_MSG(m, x.root()[0][0].empty());
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


#if 0
    // now let's mutuate one
    b.root()[0] == 33;

    IT_ASSERT(b.root()[0] == 33);
    IT_ASSERT(b.root().begin().begin()[0] == 42);
#endif
}

template <typename T>
void add_recursive(T x, unsigned int n) {
    auto y = x.emplace(n);
    if (n > 0) add_recursive(y, n - 1);
}

void create_complicated(ict::multivector<int> & tree) {
    add_recursive(tree.root(), 3);
    add_recursive(tree.root(), 3);
    add_recursive(tree.root(), 3);

    tree.root().emplace(33).emplace(34).emplace(35);

    int c = 4;
    for (auto n : {1, 2, 3}) {
        tree.root().emplace(n);
        auto i = tree.root().end() - 1;
        for (int n = 0; n < 3; ++n) {
            i.emplace(c++);
            auto j = i.begin() + i.size() - 1;
            for (int n = 0; n < 2; ++n) j.emplace(n);
        }
    }
}

ict::multivector<int> create_complicated() {
    ict::multivector<int> comp;
    create_complicated(comp);
    return comp;
}

void multivector_unit::complicated() {

    ict::multivector<int> a = create_complicated();

    IT_ASSERT(a.size() == 45);

    //IT_ASSERT(a.root().child_count() == 45);

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
    IT_ASSERT_MSG(x, x ==
    "3 {2 {1 {0}}} 3 {2 {1 {0}}} 3 {2 {1 {0}}} 33 {34 {35}} 1 {4 {0 1} 5 {0 1} 6 {0 1}} 2 {7 {0 1} 8 {0 1} 9 {0 1}} 3 {10 {0 1} 11 {0 1} 12 {0 1}}");

    *c.root()[0] = 1;

    x = ict::compact_string(c);
    IT_ASSERT_MSG(x, x ==
    "1 {2 {1 {0}}} 3 {2 {1 {0}}} 3 {2 {1 {0}}} 33 {34 {35}} 1 {4 {0 1} 5 {0 1} 6 {0 1}} 2 {7 {0 1} 8 {0 1} 9 {0 1}} 3 {10 {0 1} 11 {0 1} 12 {0 1}}");
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
    boo() {
        //IT_WARN("default constructor");
    }
    ~boo() {
        //IT_WARN("destroying " << name);
    }
    boo(int n, const ict::bitstring & bits, const std::string & name) : n(n), bits(bits), name(name) {
        //IT_WARN("constructor " << name);
    }
    boo(const boo& b) : n(b.n), bits(b.bits), name(b.name) {
        //IT_WARN("copying " << name);
    }
    int n;
    ict::bitstring bits;
    std::string name;
    bool operator==(const boo & b) const {
        return n == b.n && bits == b.bits && name == b.name;
    }
};


std::ostream & operator<<(std::ostream & os, const boo & b) {
    os << b.n << ' ' << b.bits << ' ' << b.name;
    return os;
}

void multivector_unit::non_pod() {
    ict::multivector<boo> tree;
    //IT_WARN("A");
    tree.root().emplace_back(boo{ 0, "@1", "mark" });
    tree.root().emplace_back(boo{ 1, "@11", "allan" });
    tree.root().emplace_back(boo{ 2, "@111", "beckwith" });
    tree.root().emplace_back(boo{ 3, "@1111", "phil" });
    //IT_WARN("B");
    tree.clear();
    //IT_WARN("C");
    tree.root().emplace(0, "@1", "mark");
    tree.root().emplace(1, "@11", "allan");
    tree.root().emplace(2, "@111", "beckwith");
    tree.root().emplace(3, "@1111", "phil");
    //IT_WARN("D");

    auto i = tree.root().begin();
    IT_ASSERT(i->n == 0);
    IT_ASSERT(i->bits == "@1");
    IT_ASSERT(i->name == "mark");
    IT_ASSERT(*i == (boo{ 0, "@1", "mark" }));

    auto j = i;
    IT_ASSERT(j->n == 0);
    IT_ASSERT(j->bits == "@1");
    IT_ASSERT(j->name == "mark");
    IT_ASSERT(*j == (boo{ 0, "@1", "mark" }));

    auto b = boo{4, "@0", "joe"};
    *j = b;
    IT_ASSERT(j->n == 4);
    IT_ASSERT(j->bits == "@0");
    IT_ASSERT(j->name == "joe");
    IT_ASSERT(*j == (boo{ 4, "@0", "joe" }));

    IT_ASSERT(i->n == 4);
    IT_ASSERT(i->bits == "@0");
    IT_ASSERT(i->name == "joe");
    IT_ASSERT(*i == (boo{ 4, "@0", "joe" }));
    //IT_WARN('\n' << tree);
}

struct Custom {
    Custom() {}
    Custom(const std::string & name, int length, uint64_t value, const std::string & description) :
        name(name), length(length), value(value), description(description) {}
    std::string name;
    int length;
    uint64_t value; 
    std::string description;
    bool operator==(const std::string & n) const { return name == n; }
    bool operator==(const std::string & n) { return name == n; }
};

std::string name_of(const Custom & value) 
{ 
    //IT_WARN("getting name_of");
    return value.name; }

std::string to_string(const Custom & value) { return value.name; }


std::ostream & operator<<(std::ostream & os, const Custom & c) {
    os << c.name << " " << c.length << " " << c.value << " " << c.description;
    return os;
}

#if 0
void generate_from_node(ict::multivector<Custom>::cursor c, IT::Node node) {
    for (auto n = node; !n.empty(); n = n.next()) {
        auto i = c.emplace(n.name(), n.length(), n, n.description());
        if (!n.child().empty()) generate_from_node(i, n.child());
    }
}
#endif

void multivector_unit::generate() {
#if 0
    IT::Spec spec("icd.xddl");
    IT::Message m(spec);

    m = IT::BitString("0300022220000101391799aeebb3680741110bf600f1100001010000000102e0e000140201d011270e8080210a0100000a8106000000005200f11000013103c540f4");

    IT_ASSERT(!m.empty());
    ict::multivector<Custom> msg;
    generate_from_node(msg.root(), m.root().child());
    ict::verify(msg);

    //std::cout << ict::to_text(msg) << "\n";

    auto j = ict::find(msg.root(), "c1");
    IT_ASSERT(j != msg.root().end());
    IT_ASSERT(j->name == "c1");

    auto a = ict::find(msg.root(), "TimestampIncl");
    IT_ASSERT(a != msg.root().end());
    IT_ASSERT(a->name == "TimestampIncl");
    IT_ASSERT(*a == "TimestampIncl");
    
    a = ict::find(msg.root(), ict::path("/TimestampIncl"));
    IT_ASSERT_MSG(ict::path("/TimestampIncl"), a != msg.root().end());
    IT_ASSERT_MSG(a->name, a->name == "TimestampIncl");
    IT_ASSERT(*a == "TimestampIncl");
    
    a = ict::find(msg.root(), ict::path("//c1"));
    IT_ASSERT(a != msg.root().end());
    IT_ASSERT(a->name == "c1");
    IT_ASSERT(*a == "c1");

    a = ict::find(msg.root(), "not_here");
    IT_ASSERT(a == msg.root().end());

    a = ict::find(msg.root(), ict::path("//rrc-TransactionIdentifier"));
    IT_ASSERT(a != msg.root().end());
    IT_ASSERT(*a == "rrc-TransactionIdentifier");

    a = ict::find(msg.root(), ict::path("//c1/choice"));
    IT_ASSERT(a != msg.root().end());
    IT_ASSERT(*a == "choice");
#endif
}

void multivector_unit::initializer_list() {
   auto tree = ict::multivector<int>{ 1, 2, {4, 5, 6, {7, 8, 9}}, 5, {7}, 3 };
   IT_ASSERT(ict::compact_string(tree) == "1 2 {4 5 6 {7 8 9}} 5 {7} 3");

   auto tree2 = ict::multivector<int>{ 1, 2, {{4, 5, 6, {7, 8, 9}}}, 5, {7}, 3 };
   IT_ASSERT(ict::compact_string(tree2) == "1 2 {0 {4 5 6 {7 8 9}}} 5 {7} 3");
}

template <typename Cursor>
Cursor test_path(Cursor parent, const std::string & path_string, const std::string & result) {
    auto p = ict::path(path_string);
    //IT_WARN("testing " << path_string << " : " << p);
    auto i = ict::find(parent, p);
    //auto i = ict::xfind(parent, p, [](const std::string & a, const std::string & b){ return a == b; });
    if (result.empty()) {
        IT_ASSERT_MSG(p << " not expected to be found starting at " << *parent, i == parent.end());
    } else {
        IT_ASSERT_MSG(p << " expected to be found starting at " << *parent, i != parent.end());
        IT_ASSERT(*i == result);
    }
    return i;
}

template <typename Cursor>
Cursor test_rpath(Cursor first, const std::string & path_string, const std::string & result) {
    auto p = ict::path(path_string);
    auto i = ict::rfind(first, p);
    if (result.empty()) {
        IT_ASSERT_MSG("\"" << *i <<"\" not expected to be found starting at " << *first << " with path " << p, i.is_root());
    } else {
        IT_ASSERT_MSG(p << " expected to be found starting at " << *first, !i.is_root());
        IT_ASSERT(*i == result);
    }
    return i;
}

void multivector_unit::find_find_find() {
    {
        auto tree = ict::multivector<std::string> { "one", "two", { "four", "five" }, "three" };
        //IT_WARN('\n' << ict::to_text(tree));
        auto i = ict::find(tree.root(), "two/four");
        IT_ASSERT(i != tree.root().end());
        IT_ASSERT(*i == "four");
        i = ict::find(tree.root(), "two/five");
        IT_ASSERT(i != tree.root().end());
        IT_ASSERT(*i == "five");
        i = ict::find(tree.root(), "six");
        IT_ASSERT_MSG(*i, i == tree.root().end());
        i = ict::find(tree.root(), "two/six");
        IT_ASSERT_MSG(*i, i == tree.root().end());
        i = ict::find(tree.root(), "two/three");
        IT_ASSERT_MSG(*i, i == tree.root().end());
    }    
    {
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

        //IT_WARN('\n' << ict::to_text(tree));
        test_path(tree.root(), "two", "two");
        test_path(tree.root(), "two/four", "four");
        test_path(tree.root(), "two/four", "four");
        test_path(tree.root(), "two/six", "");
        test_path(tree.root(), "two/four/one", "");
        test_path(tree.root(), "two/five/one", "one");
        test_path(tree.root(), "two/five/two", "two");
        auto c = test_path(tree.root(), "two/five/two/seven", "seven");
        auto r = ict::get_root(c);
        IT_ASSERT(r.is_root());
        test_rpath(c, "five", "five");
        test_rpath(c, "five/one", "one");
        test_rpath(c, "five/six", "");
        test_rpath(c, "two/four", "four");

        test_path(tree.root(), "two/five/two/seven/three", "");

    }
}

void multivector_unit::find_if() {
    auto tree = ict::multivector<std::string> 
    { "one", 
      "two", 
      { "four", 
        "five", 
        { "one", 
          "two", 
          "wow", 
          { "six", 
            "seven" 
          }, 
          "three" 
        }
      }, 
      "three" 
    };
    auto c = ict::find(tree.root(), "three");
    IT_ASSERT(c != tree.root().end());
    IT_ASSERT(*c == "three");

    c = ict::find(tree.root(), "two/five");
    IT_ASSERT(c != tree.root().end());
    IT_ASSERT(*c == "five");

    c = ict::find(tree.root(), "two/five/wow");
    IT_ASSERT(c != tree.root().end());
    IT_ASSERT(*c == "wow");

    c = ict::find(tree.root(), "two/five/wow/seven");
    IT_ASSERT(c != tree.root().end());
    IT_ASSERT(*c == "seven");
}

void multivector_unit::totally_ordered() {
    auto a = ict::multivector<std::string>{ "a" };
    auto b = ict::multivector<std::string>{ "b" };
    IT_ASSERT(a < b);
    auto c = ict::multivector<std::string>{ "a", "b" };
    IT_ASSERT(a < c);
    IT_ASSERT(b > c);

    auto a1 = ict::multivector<std::string>{ "a", { "b" } };
    IT_ASSERT(a < a1);
    auto a2 = ict::multivector<std::string>{ "a", { "c" } };
    IT_ASSERT(a1 < a2);
    IT_ASSERT(a2 > a1);
    IT_ASSERT(a2 != a1);
    auto a3 = ict::multivector<std::string>{ "a", { "b", "c", "d", { "e", "f", "g" } } };
    auto a4 = ict::multivector<std::string>{ "a", { "b", "c", "d", { "e", "f", "g" } } };
    IT_ASSERT(a3 == a4);
    auto i = ict::find(a4.root(), "a/d/g");
    IT_ASSERT(i != a4.root().end());
    IT_ASSERT(*i == "g");
    *i = "h";
    IT_ASSERT(a3 != a4);
    IT_ASSERT(a3 < a4);
}

void multivector_unit::moving() {
    auto a3 = ict::multivector<std::string>{ "a", { "b", "c", "d", { "e", "f", "g" } } };
}

template <typename T>
void compare_linear(const ict::multivector<T> & a, const std::vector<T> b) {
    typedef typename ict::multivector<T>::const_linear_cursor linear_type;
    auto first = ict::linear_begin(a.root());
    auto last = ict::linear_end(a.root());
    IT_ASSERT(std::equal(first, last, b.begin()));
}

void multivector_unit::linear() {
    auto a1 = ict::multivector<std::string>{ "a", "b", "c" };
    auto v1 = std::vector<std::string>{ "a", "b", "c" };
    compare_linear(a1, v1);

    a1 = ict::multivector<std::string>{ "a", { "b" }, "c" };
    compare_linear(a1, v1);

    a1 = ict::multivector<std::string>{ "a", "b", { "c" } };
    compare_linear(a1, v1);

    a1 = ict::multivector<std::string>{ "a", { "b", "c" } };
    compare_linear(a1, v1);

    a1 = ict::multivector<std::string>{ "a", { "b", { "c" } } };
    compare_linear(a1, v1);
}

void multivector_unit::promote() {
    {
        auto a = ict::multivector<int>{ 1, 2, 3, { 10, 11, 12 } };
        auto b = ict::multivector<int>{ 1, 2, 10, 11, 12 };

        IT_ASSERT(a != b);

        ict::promote_last(a.root());
        ict::verify(a);

        IT_ASSERT(a == b);
    }
    {
        auto a = ict::multivector<int>{ 1, 2, 3, { 10, 11, 12, { 100, 101, 102}} };
        auto b = ict::multivector<int>{ 1, 2, 10, 11, 12, { 100, 101, 102}};

        IT_ASSERT(a != b);
        a.root().promote_last();
        ict::verify(a);

        IT_ASSERT_MSG(ict::to_text(a), a == b);
    }
    {
        auto a = ict::multivector<int>{ 1, 2, 3, { 10, 11, 12, { 100, 101, 102, { 201, 202, 203 }}} };
        auto b = ict::multivector<int>{ 1, 2, 10, 11, 12, { 100, 101, 102, { 201, 202, 203 }}};

        IT_ASSERT(a != b);
        a.root().promote_last();
        ict::verify(a);

        IT_ASSERT_MSG(ict::to_text(a), a == b);
    }
}

void multivector_unit::ascending() {
    auto m = ict::multivector<int>{1, {10, { 100, 101, 102}}, 2, 3, 4};
    auto last = ict::ascending_begin(m.root()); // points to 5
    IT_ASSERT_MSG(*last, *last == 4);
    std::ostringstream os;
    while (!last.is_root()) {
        os << *last << ' ';
        ++last;
    }
    auto s = os.str();
    IT_ASSERT_MSG(s, s == "4 3 2 1 ");

}

void multivector_unit::ascending2() {
    std::ostringstream os;
    auto m = ict::multivector<int>{1, {10, { 100, 101, 102}}, 2, 3, 4};
    auto last = ict::ascending_begin(m.root()[0][0]);
    // last points to 102
    IT_ASSERT_MSG(*last, *last == 102);
    while (!last.is_root()) {
        os << *last << ' ';
        ++last;
    }
    IT_ASSERT_MSG(os.str(), os.str() == "102 101 100 10 1 ");
}


int main (int, char **) {
    multivector_unit test;
    ict::unit_test<multivector_unit> ut(&test);
    return ut.run();
}
