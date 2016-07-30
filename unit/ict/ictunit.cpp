//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/ict for license.
#include "ictunit.h"

void ict_unit::sanity()
{
    ict::url x;
    IT_ASSERT(x.empty());

    ict::url a{"a/3GPP2/TS-23.038.xddl#4d"};
    IT_ASSERT(!a.empty());

    ict::url b{"a/3GPP2/TS-23.038.xddl#4d"};
    IT_ASSERT(a == b);

    ict::url c = a;
    IT_ASSERT(c == b);
}

struct url_unit_type {
    std::string str() const {
        return path + file + anchor;
    }

    std::string path;
    std::string file;
    std::string anchor;
};

bool operator==(const ict::url & x, const url_unit_type & y) {
#if 0
    IT_WARN("testing equality");
    IT_WARN("x.path = " << x.path);
    IT_WARN("y.path = " << y.path);
    IT_WARN("x.file = " << x.file);
    IT_WARN("y.file = " << y.file);
    IT_WARN("x.anchor = " << x.anchor);
    IT_WARN("y.anchor = " << y.anchor);
#endif
    return (x.path == y.path) && (x.file == y.file) && (x.anchor == y.anchor);
}

void ict_unit::create_url()
{
    std::vector< std::pair<ict::url, url_unit_type> > url_tests = 
        { 
            { { "just_a_file" }, {"", "just_a_file", ""} },
            { { "#4d" }, {"", "", "#4d"} },
            { { "TS-23.038.xddl#4d" }, {"", "TS-23.038.xddl", "#4d"} },
            { { "3GPP2/TS-23.038.xddl#4d" }, {"3GPP2/", "TS-23.038.xddl", "#4d"} },
            { { "a/3GPP2/TS-23.038.xddl#4d" }, {"a/3GPP2/", "TS-23.038.xddl", "#4d"} },
        };

    for (const auto & i : url_tests) { // vs2014 won't compile this without this brace
        IT_ASSERT_MSG(i.first.str() << " != " << i.second.str(), i.first == i.second);
    }
}

struct relative_url_type {
    ict::url base;
    ict::url offset;
    ict::url result;
};
    

void ict_unit::relative_url() {
    std::vector<relative_url_type> url_tests = {
        { "", "" , "" },
        { "", "#3.1.1" , "#3.1.1" },
        { "", "C.R1001-G.xddl" , "C.R1001-G.xddl" },
        { "", "C.R1001-G.xddl#3.1.1" , "C.R1001-G.xddl#3.1.1" },

        { "C.S0005.xddl", "" , "C.S0005.xddl" },
        { "C.S0005.xddl", "#3.1.1" , "C.S0005.xddl#3.1.1" },
        { "C.S0005.xddl", "C.R1001-G.xddl" , "C.R1001-G.xddl" },
        { "C.S0005.xddl", "C.R1001-G.xddl#3.1.1" , "C.R1001-G.xddl#3.1.1" },

        { "3GPP2/C.S0005.xddl", "" , "3GPP2/C.S0005.xddl" },
        { "3GPP2/C.S0005.xddl", "#3.1.1" , "3GPP2/C.S0005.xddl#3.1.1" },
        { "3GPP2/C.S0005.xddl", "C.R1001-G.xddl" , "3GPP2/C.R1001-G.xddl" },
        { "3GPP2/C.S0005.xddl", "C.R1001-G.xddl#3.1.1" , "3GPP2/C.R1001-G.xddl#3.1.1" },

        { "3GPP2/C.S0005.xddl", "foo/b.xddl" , "3GPP2/foo/b.xddl" },

        { "icd.xddl", "#zero", "icd.xddl#zero" },
        { "3GPP/TS-24.008.xddl", "TS-24.007.xddl#11.2", "3GPP/TS-24.007.xddl#11.2" },
        { "3GPP/TS-24.008.xddl", "TS-24.007.xddl#11.2.2.1c", "3GPP/TS-24.007.xddl#11.2.2.1c" },
    };

    for (const auto & i : url_tests) {
        auto result = ict::relative_url(i.base, i.offset);
        IT_ASSERT_MSG(result.str() << " == " << i.result.str(), result == i.result);
    }
}

void ict_unit::osstream() {
    ict::osstream os;
    std::ostringstream sos;
    auto words = std::vector<std::string> { "zymosan", "zymosans", "zymoscope", "zymoses", "zymosimeter",
        "zymosis", "zymosterol", "zymosthenic", "zymotechnic", "zymotechnical", "zymotechnics",
        "zymotechny", "zymotic", "zymotically", "zymotize", "zymotoxic", "zymurgies",
        "zymurgy", "zythem", "zythum", "zyzzyva", "zyzzyvas" };

    for (auto & i : words) {
        os << i;
        sos << i;
    }
    IT_ASSERT(os.str() == sos.str());
}

int main (int, char **)
{
    ict_unit test;
    ict::unit_test<ict_unit> ut(&test);
    return ut.run();
}
