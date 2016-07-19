#pragma once
//-- Copyright 2015 Intrig
//-- see https://github.com/intrig/xenon for license

#include <ict/unit.h>

class expr_unit 
{
    public:
    void register_tests(ict::unit_test<expr_unit> & ut) {
        ut.add(&expr_unit::expr_sanity);
        ut.add(&expr_unit::expr_fail);
    }

    /* Tests */
    void expr_sanity();
    void expr_fail();
};
