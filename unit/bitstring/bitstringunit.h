#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/ict for license.
#include <unit.h>

namespace ict {
class bitstring_unit 
{
    public:
    void register_tests(unit_test<bitstring_unit> & ut) 
    {
        ut.skip();
        ut.cont();

        ut.add(&bitstring_unit::toUint32);
        ut.add(&bitstring_unit::endian);
        ut.add(&bitstring_unit::expand_endian);
        ut.add(&bitstring_unit::toUint64);
        ut.add(&bitstring_unit::fromNumber);
        ut.add(&bitstring_unit::toInteger);

        ut.add(&bitstring_unit::bitstring_sanity);
        ut.add(&bitstring_unit::obs);
        ut.add(&bitstring_unit::ibs);
        ut.add(&bitstring_unit::ibs_constraint);
        ut.add(&bitstring_unit::from_string);
        ut.add(&bitstring_unit::int_convert);
        ut.add(&bitstring_unit::modern_replace);
        ut.add(&bitstring_unit::modern_pad);
        ut.add(&bitstring_unit::modern_gsm7);
        ut.add(&bitstring_unit::modern_sms_difficult);

        ut.add(&bitstring_unit::iterators);

        ut.skip();
        ut.cont();
    }

    void toUint32();
    void endian();
    void expand_endian();
    void toUint64();
    void fromNumber();
    void toInteger();

    /* Tests */

    void bitstring_sanity();
    void obs();
    void ibs();
    void ibs_constraint();
    void from_string();
    void int_convert();
    void modern_pad();
    void modern_replace();
    void modern_gsm7();
    void modern_sms_difficult();
    void iterators();
};
}
