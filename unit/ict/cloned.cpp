#include "ictunit.h"
#include <cloned_ptr.h>
#include <multivector.h>

struct base_type {
    base_type() {}
    virtual ~base_type();
    base_type(const base_type & x) : i(x.i) {}
    base_type & operator=(const base_type & x) { i = x.i; return *this; }
    virtual std::string name() const { return "base_type"; }
    virtual base_type * clone() const { 
        //IT_WARN("cloning base_type");
        return new base_type(*this); }
    int i = 42;
};

base_type::~base_type() {}

struct derived_a : public base_type {
    derived_a() : base_type() {}
    virtual ~derived_a();
    derived_a(const derived_a & x) : base_type(x), j(x.j) {}
    derived_a & operator=(const derived_a & x) { 
        base_type::operator=(x);
        j = x.j; 
        return *this; }
    virtual std::string name() const { return "derived_a"; }
    virtual base_type * clone() const { 
        //IT_WARN("cloning derived_a");
        return new derived_a(*this); }
    int j = 33;
};

derived_a::~derived_a() {}

struct derived_b : public derived_a {
    derived_b() : derived_a() {}
    virtual ~derived_b();
    derived_b(const derived_b & x) : derived_a(x), k(x.k) {}
    derived_b & operator=(const derived_b & x) { 
        derived_a::operator=(x);
        k = x.k; 
        return *this; }
    virtual std::string name() const { return "derived_b"; }
    virtual base_type * clone() const { 
        //IT_WARN("cloning derived_b");
        return new derived_b(*this); }
    int k = 111;
};

derived_b::~derived_b() {}

void ict_unit::cloned() {
    auto b = ict::make_cloned<base_type>();
    IT_ASSERT(b->i == 42);
    IT_ASSERT((*b).i == 42);
    b->i = 2;
    IT_ASSERT(b->i == 2);
    IT_ASSERT((*b).i == 2);

}

void ict_unit::cloned_derived() {
    //IT_WARN("creating b");
    auto b = ict::make_cloned<base_type>();
    b->i = 1;
    //IT_WARN("creating b_clone");

    ict::cloned_ptr<base_type> b_clone(b);
    b_clone->i = 2;

    IT_ASSERT(b->i == 1);
    IT_ASSERT(b_clone->i == 2);

    IT_ASSERT(b.p);
    IT_ASSERT(b_clone.p);
    IT_ASSERT(b.p != b_clone.p);
    IT_ASSERT(b_clone->i == 2);
    auto d1 = ict::make_cloned<derived_a>();
    IT_ASSERT(d1->i == 42);
    IT_ASSERT(d1->j == 33);
}

void ict_unit::cloned_vector() {
    std::vector<ict::cloned_ptr<base_type>> items;
    items.reserve(50);

    items.emplace_back(ict::make_cloned<base_type>());
    items.emplace_back(ict::make_cloned<derived_a>());

    for (int i = 0; i < 10; ++ i) {
        items.emplace_back(ict::make_cloned<derived_b>());
        items.back()->i = i;
    }
    auto items2 = items;
    for (auto & item : items2) item->i = 99;

    for (int i = 0; i < 10; ++ i) IT_ASSERT(items[i + 2]->i == i);
    for (auto & item : items2) { IT_ASSERT(item->i == 99); }
}

typedef ict::cloned_ptr<base_type> pointer_type;
namespace ict {
    std::string to_string(const pointer_type & x);
    std::ostream& operator<<(std::ostream& os, const pointer_type &x);

    std::string to_string(const pointer_type & x) {
        return x->name();
    }

    std::ostream& operator<<(std::ostream& os, const pointer_type &x) {
        os << x->name();
        return os;
    }
}


void ict_unit::cloned_multivector() {
    ict::multivector<pointer_type> tree;
    auto e = ict::make_cloned<derived_a>();
    tree.root().emplace_back(e);
    IT_ASSERT(ict::to_text(tree) == "derived_a\n");
}
