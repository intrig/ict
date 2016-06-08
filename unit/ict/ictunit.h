#pragma once
#include <ict/unit.h>

class ict_unit 
{
    public:
    void register_tests(ict::unit_test<ict_unit> & ut) {
        ut.skip();
        ut.cont();
        ut.add(&ict_unit::sanity);
        ut.add(&ict_unit::create_url);
        ut.add(&ict_unit::relative_url);
        ut.add(&ict_unit::cloned);
        ut.add(&ict_unit::cloned_derived);
        ut.add(&ict_unit::cloned_vector);
        ut.add(&ict_unit::cloned_multivector);
    }

    void sanity();
    void create_url();
    void relative_url();
    void cloned();
    void cloned_derived();
    void cloned_vector();
    void cloned_multivector();
};
