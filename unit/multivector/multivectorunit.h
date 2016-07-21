#pragma once
#include <unit.h>

class multivector_unit 
{
    public:
    void register_tests(ict::unit_test<multivector_unit> & ut) {
        ut.skip();
        ut.cont();
        ut.add(&multivector_unit::empty_multivectors);
        ut.add(&multivector_unit::adding);
        ut.add(&multivector_unit::size_one);
        ut.add(&multivector_unit::size_two);
        ut.add(&multivector_unit::complicated);
        ut.add(&multivector_unit::iterators);
        ut.add(&multivector_unit::non_pod);
        ut.add(&multivector_unit::initializer_list);
        ut.add(&multivector_unit::totally_ordered);
        ut.add(&multivector_unit::moving);
        ut.add(&multivector_unit::linear);
        ut.add(&multivector_unit::promote);
        ut.add(&multivector_unit::ascending);
        ut.add(&multivector_unit::ascending2);
        ut.add(&multivector_unit::append_children);
    }

    void empty_multivectors();
    void adding();
    void size_one();
    void size_two();
    void complicated();
    void iterators();
    void non_pod();
    void initializer_list();
    void totally_ordered();
    void moving();
    void linear();
    void promote();
    void ascending();
    void ascending2();
    void append_children();
};
