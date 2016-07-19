//-- Copyright 2015 Intrig
//-- See https://github.com/intrig/xenon for license.
#include "iexprunit.h"

#include <ict/expr.h>

#include <vector>

struct test_result {
    const char *expr;
    int64_t result;
};

std::vector<test_result> basic_tests = {
#if 0
#endif
    { "0", 0 },
    { "5 + 3", 8 },
    { "0 - 3", -3 },
    { "4 / 3", 1 },
    { "(5 + 1) / 2", 3 },
    { "foo * 2", 46 },
    { "boo / 3", 3 },
    { "3 + goo", 5 },
    { "goo + 3", 5 },
    { "8 % 7", 1 },

    { "5 << 2", 20 },
    { "5 < 5", 0 },
    { "#10 << 1", 0x20 },

    { "#10 ls 1", 0x20 },
    { "#ffff << 8", 0xffff00 },
    { "@0010 << 2", 8 },

    { "5 <= 5", 1 },
    { "5 lte 5", 1 },
    { "5 >= 5", 1 },
    { "5 gte 5", 1 },
    { "6 > 7", 0 },

    { "5 >> 1", 2 },
    { "5 rs 1", 2 },

    { "5 > 2", 1},
    { "5 < 2", 0},
    { "5 gt 2", 1},
    { "5 lt 2", 0},
    { "5 == 5", 1},
    { "5 eq 5", 1},
    { "5 != 5", 0},
    { "23 == 12", 0 },
    { "23 != 12", 1 },
    { "goo == 2", 1 },

    { "1 || 0", 1 },
    { "0 || 1", 1 },
    { "1 || 1", 1 },
    { "1 or 1", 1 },

    { "1 && 0", 0 },
    { "0 && 1", 0 },
    { "1 && 1", 1 },
    { "1 and 0", 0 },

    { "2 ** 3", 8 },
     
    { "7 & 3", 3 },
    { "1 & 0", 0 },
    { "@101 & @010", 0 },
    { "@101 | @010", 7 },

    { "-3", -3 },
    { "3 - -3", 6 },
    { "3 / -3", -1 },
    { "-3 / -3", 1 },

    { "3 * 2 * 5", 30 },
    { "Length+1", 2 },
    { "Length-3", -2 },
    { "_Length-3", -2 },
    { "1 ? 23 : 0", 23 },
    { "0 ? 1 : 0", 0 },
    { "foo > 3 ? bar : 188", -15 },
    { "foo > 3 ? bar\n : 188", -15 },

    { "5 + bar + foo", 13 },
    { "5 + bar + {foo}", 13 },
    { "5 + bar + {3 -_#'\"}", 13 },
    { "5 + bar + {foo-bar}", 13 },
    { "5 + bar + {foo bar}", 13 },
    { "5 + bar + {-foo*bar}", 13 },

    { "func(1)", 1 },
    { "func(1) + 1", 2 },
    { "foo + func(1) + 1", 25 },
    { "zero()", 0 },
    { "one(1)", 1 },
    { "two(1, 2)", 2 },
    { "three(1, 2, 3)", 3 },
    { "three(foo, bar, goo)", 3 },
    { "three(one(one(1)), two(1, one(1)), 25)", 3 },
    { "add(foo, bar, goo)", 23 - 15 + 2 },
    { "defined('foo')", 42 },


    { "foo.bar", 1},
    { "{foo}", 23},
    { "{foo}.bar", 1},
    { "{foo}.{bar}", 1},
    { "!foo", 0 },
    { "!0", 1 },
    { "!1", 0 },
};

struct context_type {
    int i = 1;
};

typedef ict::expr_type<int64_t>::param_type param_type;

inline int64_t eval_function(const std::string & name, context_type, const std::vector<param_type> &params) {
    if (name == "add") {
        int64_t r = 0;
        for_each(params.begin(), params.end(), [&](const param_type & v){r += v.number; });
        return r;
    } else if (name == "defined") {
        IT_ASSERT(params.size() == 1);
        IT_ASSERT(params[0].name == "foo");
        return 42;
    }
    return params.size();
}

inline int64_t eval_variable(const std::string & name, context_type) { 
    if (name == "foo") return 23;
    if (name == "boo") return 9;
    if (name == "goo") return 2;
    if (name == "bar") return -15;
    if (name == "Length") return 1;
    if (name == "_Length") return 1;
    if (name == "3 -_#'\"") return 23;
    if (name =="foo-bar") return 23;
    if (name =="foo bar") return 23;
    if (name =="-foo*bar") return 23;
    IT_PANIC("cannot evaluate variable \"" << name << "\"");
}

inline int64_t eval_variable_list(const std::string &, const std::string &, context_type) { 
    return 1;
}

void expr_unit::expr_sanity() {

    // the same for now, but don't have to be
    context_type compile_context;
    context_type runtime_context;
    for (auto & i : basic_tests) {
        try {
            ict::expr_type<int64_t> e(i.expr, compile_context);
            auto r = e.value(runtime_context);
            IT_ASSERT_MSG(i.expr << " == " << r, r == i.result);
        } catch (ict::exception & e) {
            IT_FORCE_ASSERT(e.what() << " in \"" << i.expr << "\"");
        }
    }
}

struct fail_result {
    const char * expr;
    bool fail;
};

std::vector<fail_result> fail_tests = {
    { "3foo", true },
    { "3 <", true },
    { "3 <<", true },
    { "3 <==", true },
    { "3 <>", true },
    { "#z", true }
};

void expr_unit::expr_fail() {
    context_type compile_context;
    context_type runtime_context;
    for (auto & i : fail_tests) {
        try {
            ict::expr_type<int64_t> e(i.expr, compile_context);
            e.value(runtime_context);
            // should be here if it was flagged as pass.
            if (i.fail == true) IT_FORCE_ASSERT(i.expr << " should have failed");
        } catch (ict::exception & e) {
            // should be here if it was flagged as fail.
            if (i.fail == false) IT_FORCE_ASSERT(e.what() << " in \"" << i.expr << "\"");
        }
    }
}

int main() {
    expr_unit test;
    ict::unit_test<expr_unit> ut(&test);
    return ut.run();
}
