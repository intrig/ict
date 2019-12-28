#include <assert.h>
#include <command.h>
#include <fstream>
#include <iostream>
#include <multivector.h>
#include <string>

template <typename T> void add_recursive(T x, unsigned int n) {
    auto y = x.emplace(n);
    if (n > 0)
        add_recursive(y, n - 1);
}

static ict::multivector<int> one() {
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
    for (auto x : {1, 2, 3}) {
        tree.root().emplace_back(x);
        auto i = tree.root().end() - 1;
        for (int y = 0; y < 3; ++y) {
            i.emplace_back(c++);
            auto j = i.begin() + i.size() - 1;
            //--j;
            for (int z = 0; z < 2; ++z)
                j.emplace_back(z);
        }
    }
    return tree;
}

static ict::multivector<int> two() {
    ict::multivector<int> tree;
    for (int i = 0; i < 10; ++i)
        tree.root().emplace_back(i);
    for (int i = 40; i < 50; ++i)
        (tree.begin() + 5).emplace_back(i);
    for (int i = 100; i < 103; ++i)
        ((tree.begin() + 5).begin() + 3).emplace_back(i);
    for (int i = 0; i < 10; ++i)
        ((tree.begin() + 5).begin() + 7).emplace_back(i);
    ict::verify(tree);
    return tree;
}

static bool grow(ict::multivector<int> &tree) {
    for (int i = 200; i < 240; ++i)
        (tree.begin() + 5).emplace_back(i);
    ict::verify(tree);
    return true;
}

static void iterate() {
    ict::multivector<int> tree = two();

    std::cout << "print recursively using cursor\n";
    std::cout << ict::to_text(tree);

    std::cout << "\nprint top level nodes only\n";
    for (int i : tree.root())
        std::cout << i << "\n";

    std::cout << "\nprint all items using linear cursor:\n";
    for (ict::multivector<int>::linear_cursor i = tree.begin(); i != tree.end();
         ++i)
        std::cout << *i << "\n";

    std::cout << "\nagain, using range for loop:\n";
    for (int i : tree)
        std::cout << i << "\n";

    std::cout << "\nprint first level children of tree[5] only\n";
    for (int i : (tree.begin() + 5))
        std::cout << i << "\n";

    std::cout << "\nagain, using linear cursor:\n";
    for (ict::multivector<int>::linear_cursor i = tree.begin(); i != tree.end();
         ++i)
        std::cout << *i << "\n";

    std::cout
        << "\nprint all children (including subchildren) of tree[5] only\n";
    auto fifth = tree.begin() + 5;
    for (ict::multivector<int>::linear_cursor i = fifth.begin();
         i != fifth.end(); ++i)
        std::cout << *i << "\n";

    std::cout << "\nreverse cursor over nodes from tree[5][7][9] on up:\n";
    auto s = --(((tree.begin() + 5).begin() + 7).end());
    for (auto i = ict::to_ascending(s); !i.is_root(); ++i)
        std::cout << *i << "\n";
}

static void basics() {
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
    for (auto i : tree.root())
        std::cout << i << "\n";

    std::cout << "\nprint tree in compact string representation:\n";
    std::cout << ict::compact_string(tree) << "\n";

    {
        auto tree2 = ict::multivector<int>{1, 2, 3, {1, 2, 3, {1, 2, 3}}, 4};
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

    if (a == b)
        std::cout << "equal\n";
    if (a != b)
        std::cout << "not equal\n";

    std::cout << "*a is " << *a << "\n";

    ++a;
    (void)a++;
    *a++;
    std::swap(a, b);
}

static void strings() {
    std::cout << "\nnow create a multivector of strings:\n";
    ict::multivector<std::string> smultivector;
    auto cursor =
        smultivector.root().emplace("one").emplace("two").emplace("three");
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
    bool operator==(const std::string &n) const { return name == n; }
    bool operator==(const std::string &n) { return name == n; }
};

static std::ostream &operator<<(std::ostream &os, const Custom &c) {
    os << c.name << " " << c.length << " " << c.value << " " << c.description;
    return os;
}

static void custom() {
    ict::multivector<Custom> tree;

    auto i = tree.root().emplace(Custom{"record", 5, 5, "0"});

    i.emplace_back(Custom{"bar", 1, 6, "a"});
    i.emplace_back(Custom{"goo", 8, 23, "b"});
    i.emplace_back(Custom{"foo", 32, 154, "c"});
    tree.root().emplace_back(Custom{"crc", 80, 5555555, "d"});

    std::cout << "\nnow create custom message:\n";
    std::cout << ict::to_text(tree) << "\n";

    auto sub = ict::multivector<Custom>(tree.begin());
    std::cout << "\nnow create a sub message:\n";
    std::cout << ict::to_text(sub) << "\n";
}

template <typename T> void print_reverse_cursor(T c) {
    while (!c.is_root()) {
        std::cout << *c << "\n";
        ++c;
    }
}

static void constmultivector(const ict::multivector<int> &tree) {
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

static void constness() {
    ict::multivector<int> tree = two();
    std::cout << ict::to_text(tree);
    constmultivector(tree);
}

int main(int argc, char **argv) {
    try {
        ict::command line("multivectorcli", "ict::multivector exerciser",
                          "mvcli [options]");
        line.add(ict::option("basics", 'b', "Display basic multivector calls",
                             [] { basics(); }));
        line.add(ict::option("iterate", 'i', "Display iterator calls",
                             [] { iterate(); }));
        line.add(ict::option("strings", 's', "Display string multivector calls",
                             [] { strings(); }));
        line.add(ict::option("custom", 'c', "Display custom multivector calls",
                             [] { custom(); }));
        line.add(ict::option("constness", 'C', "const iterators/cursors",
                             [] { constness(); }));
        line.add(ict::option("All", 'A', "run all", [] {
            basics();
            iterate();
            strings();
            custom();
            constness();
        }));

        line.parse(argc, argv);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}
