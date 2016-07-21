//-- Copyright 2015 Intrig
//-- See https://github.com/intrig/xenon for license.
#include <string>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <string>

#include <multivector.h>
#include <command.h>

template <typename T>
void add_recursive(T x, unsigned int n) {
    auto y = x.emplace(n);
    if (n > 0) add_recursive(y, n - 1);
}

ict::multivector<int> one() {
    ict::multivector<int> tree;
    ict::verify(tree);

    add_recursive(tree.root(), 3);

    ict::verify(tree);

    add_recursive(tree.root(), 3);
    ict::verify(tree);

    add_recursive(tree.root(), 3);
    ict::verify(tree);

    tree.root().emplace(33).emplace(34).emplace(35);

    int c = 4;
    for (auto n : {1, 2, 3}) {
        tree.root().emplace_back(n);
        auto i = tree.root().end() - 1;
        for (int n = 0; n < 3; ++n) {
            i.emplace_back(c++);
            auto j = i.begin() + i.size() - 1;
            //--j;
            for (int n = 0; n < 2; ++n) j.emplace_back(n);
        }
    }
    return tree;
}

#if 0
// something like this would be cool
template <typename Cursor> 
Cursor navigate(Cursor parent, Args...) {
}
#endif

ict::multivector<int> two() {
    ict::multivector<int> tree;
    for (int i = 0; i < 10; ++i) tree.root().emplace_back(i);
    for (int i = 40; i < 50; ++i) (tree.begin() + 5).emplace_back(i);
    for (int i = 100; i < 103; ++ i) ((tree.begin() + 5).begin() + 3).emplace_back(i);
    for (int i = 0; i < 10; ++i) ((tree.begin() + 5).begin() + 7).emplace_back(i);
    ict::verify(tree);
    return tree;
}

bool grow(ict::multivector<int> & tree) {
    for (int i = 200; i < 240; ++i) (tree.begin() + 5).emplace_back(i);
    ict::verify(tree);
    return true;
}

void iterate() {
    ict::multivector<int> tree = two();

    std::cout << "print recursively using cursor\n";
    std::cout << ict::to_text(tree);

    std::cout<< "\nprint top level nodes only\n";
    for (int i : tree.root()) std::cout << i << "\n";

    std::cout << "\nprint all items using linear cursor:\n";
    for (ict::multivector<int>::linear_cursor i = tree.begin(); i != tree.end(); ++i) std::cout << *i << "\n";

    std::cout << "\nagain, using range for loop:\n";
    for (int i : tree) std::cout << i << "\n";

    std::cout<< "\nprint first level children of tree[5] only\n";
    for (int i : (tree.begin() + 5)) std::cout << i << "\n";

    std::cout << "\nagain, using linear cursor:\n";
    for (ict::multivector<int>::linear_cursor i = tree.begin(); i != tree.end(); ++i) std::cout << *i << "\n";

    std::cout<< "\nprint all children (including subchildren) of tree[5] only\n";
    auto fifth = tree.begin() + 5;
    for (ict::multivector<int>::linear_cursor i = fifth.begin(); i!=fifth.end(); ++i) std::cout << *i << "\n";

    std::cout << "\nreverse cursor over nodes from tree[5][7][9] on up:\n";
    auto s = --(((tree.begin() + 5).begin() + 7).end());
    for (auto i = ict::to_ascending(s); !i.is_root(); ++i) std::cout << *i << "\n";
}

void basics() {
    ict::multivector<int> tree = one();
    ict::verify(tree);

    {
        ict::multivector<int>::cursor a;
        a = tree.begin();
        ict::verify(a);
        ict::multivector<int>::cursor b;
        b = a;
        ict::verify(b);
    }


    tree = two(); 
    ict::verify(tree);

    std::cout << ict::to_text(tree) << "\n\n";

    std::cout << "iterator over top level nodes only:\n";
    for (auto i : tree.root()) std::cout << i << "\n";

    std::cout << "\nprint tree in compact string representation:\n";
    std::cout << ict::compact_string(tree) << "\n";

    {
        auto tree2 = ict::multivector<int> { 1, 2, 3, { 1, 2, 3, { 1, 2, 3 }}, 4 };
        std::cout << "\nprint another one:\n";
        std::cout << ict::compact_string(tree2) << "\n";
    }

    std::cout << "\ncause reallocation of tree[5] and print:\n";
    grow(tree);
    std::cout << ict::to_text(tree) << "\n\n";
    ict::verify(tree);

    auto a = tree.begin();
    ict::multivector<int>::cursor b;
    b = a;
    ict::multivector<int>::cursor c(a);

    if (a == b) std::cout << "equal\n";
    if (a != b) std::cout << "not equal\n";

    std::cout << "*a is " << *a << "\n";
    
    ++a;
    (void)a++;
    *a++;
    std::swap(a, b);

#if 0
    std::cout << "\nfind 102:\n";
    auto i = std::find(tree.begin(), tree.end(), 102);
    if (i != tree.end()) std::cout << "found " << *i << "\n";

    std::cout << "\nfind 99:\n";
    i = std::find(tree.begin(), tree.end(), 99);
    if (i == tree.end()) std::cout << "not found\n";

    std::cout << "\nfind 43 with reverse cursor starting at tree[5][7][3]\n";
    auto r = ict::rfind(tree.root()[5][7][3], "43");
    if (!r.is_root()) std::cout << "found " << *r << "\n";
#endif
}

void strings() {
    std::cout << "\nnow create a multivector of strings:\n";
    ict::multivector<std::string> smultivector;
    auto cursor = smultivector.root().emplace("one").emplace("two").emplace("three");
    cursor.emplace_back("four");
    cursor.emplace_back("five");
    cursor.emplace_back("six");

    std::cout << ict::to_text(smultivector) << "\n";

    ict::verify(smultivector);
}

struct Custom {
    std::string name;
    int length;
    uint64_t value; 
    std::string description;
    bool operator==(const std::string & n) const { return name == n; }
    bool operator==(const std::string & n) { return name == n; }
};

std::ostream & operator<<(std::ostream & os, const Custom & c) {
    os << c.name << " " << c.length << " " << c.value << " " << c.description;
    return os;
}

void custom() {
    ict::multivector<Custom> tree;

    auto i = tree.root().emplace(Custom{"record", 5, 5, "0"});

    i.emplace_back(Custom{"bar", 1, 6, "a"});
    i.emplace_back(Custom{"goo", 8, 23, "b"});
    i.emplace_back(Custom{"foo", 32, 154, "c"});
    tree.root().emplace_back(Custom{"crc", 80, 5555555, "d"});

    std::cout << "\nnow create custom message:\n";
    std::cout << ict::to_text(tree) << "\n";

    ict::multivector<Custom> sub = tree.begin();
    std::cout << "\nnow create a sub message:\n";
    std::cout << ict::to_text(sub) << "\n";
}

#if 0
void generate_from_node(ict::multivector<Custom>::cursor it, IT::Node node) {
    for (auto n = node; !n.empty(); n = n.next()) {

        if (n.child().empty()) {
            it.emplace_back(Custom{ n.name(), n.length(), n, n.description()} );
        } else {
            auto i = it.emplace(Custom{ n.name(), n.length(), n, n.description()} );
            generate_from_node(i, n.child());
        }
    }
}
#endif

void generate() {
#if 0
    IT::Spec spec("icd.xddl");
    IT::Message m(spec);

    m = IT::BitString("0300022220000101391799aeebb3680741110bf600f1100001010000000102e0e000140201d011270e8080210a0100000a8106000000005200f11000013103c540f4");

    if (m.empty()) return;
    ict::multivector<Custom> msg;
    generate_from_node(msg.root(), m.root().child());

    ict::verify(msg);
    std::cout << ict::to_text(msg) << "\n";

    std::cout << "\nfind c1\n";
    auto j = std::find(msg.begin(), msg.end(), "c1");
    if (j == msg.end()) std::cout << "not found\n";
    else std::cout << "found " << *j << "\n";

    std::cout << "\nlooking for rrc-TransactionIdentifier\n";
    auto n = ict::find(msg.root(), ict::path("c1/rrcConnectionSetupComplete/rrc-TransactionIdentifier"));
    if (n !=msg.root().end()) std::cout << "found " << *n << "\n";
    else std::cout << "not found\n";

    std::cout << "\nlooking for garbage\n";
    n = ict::find(msg.root(), "garbage");
    if (n !=msg.root().end()) std::cout << "found " << *n << "\n";
    else std::cout << "not found\n";

    std::cout << "\nlooking for c1/rrcConnectionSetupComplete/garbage\n";
    auto x = ict::find(msg.root(), ict::path("c1/rrcConnectionSetupComplete/garbage"));
    if ( x!=msg.root().end()) std::cout << "found " << *x << "\n";
    else std::cout << "not found\n";

    std::cout << "\nfind Nas Mesage\n";
    auto nas = ict::find(msg.root(), ict::path("//NAS Message"));
    if (nas != msg.root().end()) {
        std::cout << "\nnow look for Message Type\n";
        auto mt = ict::find(nas, "Message type");
        if (mt != nas.end()) std::cout << "found it: " << *mt << "\n";
    }
#endif
}

void constvector(const std::vector<int> v) {
    std::vector<int>::const_iterator first = v.begin();
    std::vector<int>::const_iterator last = v.end();
    while (first != last) {
        std::cout << *first << "\n";
        ++first;
    }

}

template <typename T>
void print_reverse_cursor(T c) {
    while (!c.is_root()) {
        std::cout << *c << "\n";
        ++c;
    }
}

void constmultivector(const ict::multivector<int> & tree) {
    // forward iterator
    std::cout << "print tree using forward const_iterator\n";
    ict::verify(tree);

    auto first = tree.begin();
    auto last = tree.end();
    while (first != last) {
        std::cout << *first << "\n";
        ++first;
    }
}

void constness() {
    ict::multivector<int> tree = two();
    std::cout << ict::to_text(tree);
    constmultivector(tree);
}

int main(int argc, char **argv) {

    try {
        ict::command line("multivectorcli", "ict::multivector exerciser", "mvcli [options]");
        line.add(ict::option("basics", 'b', "Display basic multivector calls",      []{ basics(); } ));
        line.add(ict::option("iterate", 'i', "Display iterator calls",       []{ iterate(); } ));
        line.add(ict::option("strings", 's', "Display string multivector calls",    []{ strings(); } ));
        line.add(ict::option("custom", 'c', "Display custom multivector calls",     []{ custom(); } ));
        line.add(ict::option("generate", 'g', "generate advanced multivector",      []{ generate(); } ));
        line.add(ict::option("constness", 'C', "const iterators/cursors",      []{ constness(); } ));
        line.add(ict::option("All", 'A', "run all", []{ 
            basics();
            iterate();
            strings(); 
            custom();
            generate();
            constness();
        } ));

        line.parse(argc, argv);
    } catch (std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
}

