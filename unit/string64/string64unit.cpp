//-- Copyright 2015 Intrig
//-- See https://github.com/intrig/xenon for license.
#include "string64unit.h"

#include <ict/string64.h>

#include <string>

struct spaces_t {
    char const *spaced;
    char const *not_spaced;
};

const std::vector<spaces_t> spacetest = {
    { "  AABBCCDDEEFF11223344556677  ", "AABBCCDDEEFF11223344556677" },
    { "AA\tBBCCDDEEFF1122334455\t6677", "AABBCCDDEEFF11223344556677" },
    { " AA BB CC DD EE FF 11 22 33 44 55 66 77 ", "AABBCCDDEEFF11223344556677" },
    { "\t\t\tAABBCCDDEEFF112\r23344556677\t\t", "AABBCCDDEEFF11223344556677" },
    { "AABBCCDDEEFF11223344556677\r\n", "AABBCCDDEEFF11223344556677" },
    { "AABBCCDDEEFF11223344556677\n", "AABBCCDDEEFF11223344556677" },
    { "AABBCCDDEEFF11223344556677\n\r", "AABBCCDDEEFF11223344556677" },
};

void string64_unit::squash() {
    for (auto & i : spacetest) {
        std::string xstr(i.spaced);
        ict::squash(xstr);
        IT_ASSERT(xstr == i.not_spaced);
    }
}

void string64_unit::string64() {
    ict::string64 a;
    IT_ASSERT(a.empty());
    ict::string64 b = "24";
    IT_ASSERT(!b.empty());

    ict::string64 c(b);
    IT_ASSERT(!c.empty());
    IT_ASSERT(c == b);
    IT_ASSERT(b == c);

    ict::string64 d("hello");
    IT_ASSERT(d == "hello");
    IT_ASSERT(d[0] == 'h');
    IT_ASSERT(d[1] == 'e');
    IT_ASSERT(d[2] == 'l');
    IT_ASSERT(d[3] == 'l');
    IT_ASSERT(d[4] == 'o');
    IT_ASSERT(d[5] == '\0');
    IT_ASSERT(d[6] == '\0');
    IT_ASSERT(d[7] == '\0');

    IT_ASSERT(d != "goodbye");

    IT_ASSERT(d.value == 478560413032);
    uint64_t value = d.value;
    IT_ASSERT(value == 478560413032);
    c = d;
    IT_ASSERT(c == d);
    IT_ASSERT(c != a);

    {
        ict::string64 a = "";
        IT_ASSERT(a.empty());

        try {
            a = "way to long";
        } catch (ict::exception & e) {
            IT_ASSERT(std::string(e.what()).find("way to long is to long for a string64") == std::string::npos);
        }
    }

    {
        ict::string64 a = "field";
        ict::string64 b = a;

        IT_ASSERT(a == b);
    }
}

void string64_unit::string64_compare() {
    std::vector<ict::string64> v = { "a", "b", "c", "foo", "goo", "alpha", "beta", "wow", "wowo", "good", "bad" };
    auto sorted = v;
    std::sort(sorted.begin(), sorted.end());
    std::random_shuffle(v.begin(), v.end());
    auto sorted2 = v;
    std::sort(sorted2.begin(), sorted2.end());
    IT_ASSERT(sorted == sorted2);
}

int main() {
    string64_unit test;
    ict::unit_test<string64_unit> ut(&test);
    return ut.run();
}
