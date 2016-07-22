#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/ict for license.
#include <unit.h>

struct string64_unit {
    void register_tests(ict::unit_test<string64_unit> & ut) {
        ut.add(&string64_unit::squash);
        ut.add(&string64_unit::string64);
        ut.add(&string64_unit::string64_compare);
    }

    void squash();
    void string64();
    void string64_compare();
};
