#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/ict for license.

#include <unit.h>

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
