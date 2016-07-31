#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/ict for license.
#include <unit.h>

class ict_unit 
{
    public:
    void register_tests(ict::unit_test<ict_unit> & ut) {
        ut.skip();
        ut.cont();
        ut.add(&ict_unit::osstream);
        ut.add(&ict_unit::cloned);
        ut.add(&ict_unit::cloned_derived);
        ut.add(&ict_unit::cloned_vector);
        ut.add(&ict_unit::cloned_multivector);
    }

    void osstream();
    void cloned();
    void cloned_derived();
    void cloned_vector();
    void cloned_multivector();
};
