#pragma once
//-- Copyright 2015 Intrig
//-- see https://github.com/intrig/xenon for license
#include <string>
#include <cctype>
#include <iostream>
#include <map>
#include <sstream> 
#include <functional>
#include <cmath>

#include <ict/exception.h>
#include <ict/bitstring.h>

namespace ict {

    static std::vector<const char *> token_name = { 
        "NAME", "NAME_REF", "NUMBER", "BIT_SHIFT_LEFT", "BIT_SHIFT_RIGHT", "LESS_EQUAL", "GREATER_EQUAL",
        "EQUAL", "NOT", "NOT_EQUAL", "QUESTION", "COLON", "LC", "RC", "COMMA", "DOT",
        "END",
        "PLUS",   "MINUS",  "MUL",  "POW",  "DIV", "LESS", "GREATER",
        "START", "LP",     "RP",  "REM", "BIT_WISE_AND", "BIT_WISE_OR", "LOGICAL_OR", "LOGICAL_AND"
    };



template <typename T>
// T is the type of number (int64_t, double, etc.)
struct expr_type {
    enum token_id {
        NAME, NAME_REF, NUMBER, BIT_SHIFT_LEFT, BIT_SHIFT_RIGHT, 
        LESS_EQUAL, GREATER_EQUAL, EQUAL, NOT, NOT_EQUAL, 
        QUESTION, COLON, LC, RC, COMMA, DOT,
        END, PLUS, MINUS,  MUL, POW,   
        DIV, LESS, GREATER, START, LP,     
        RP,  REM, BIT_WISE_AND, BIT_WISE_OR, LOGICAL_OR, 
        LOGICAL_AND
    };

    struct token_value {
        token_value() : id(START) {}       
        token_value(token_id id) : id(id) {}       
        token_value(token_id id, T number) : id(id), number(number) {}       
        token_value(token_id id, const std::string & name) : id(id), name(name) {}       
        token_value(char ch) {
            id = static_cast<token_id>(ch);
        }
        friend bool operator==(const token_value & a, const token_id & b) { return a.id == b; }
        friend bool operator!=(const token_value & a, const token_id & b) { return !(a.id == b); }

        friend bool operator==(const token_value & a, const token_value & b) { 
            return (a.id == b.id) && (a.number == b.number) && (a.name == b.name); 
        }

        friend bool operator!=(const token_value & a, const token_value & b) { 
            return !(a == b);
        }

        std::string to_string() const {
            std::ostringstream os;
            os << token_name[id] << " (" << id << ")";
            if (id == NUMBER) os << " = " << number;
            if (id == NAME) os << " = " << name;
            return os.str();
        }
        token_id id;
        T number;
        std::string name;
    };

    struct param_type {
        std::string name;
        T number;
    };

    template <typename Context>
    struct parser {
        typedef typename std::vector<token_value>::const_iterator token_iterator;
        parser(token_iterator curr_token) : curr_token(curr_token) {}
        void compile(Context c) {
            context = c;
            while (*curr_token != END) {
                eval();
                if ((curr_token->id != END)) {
                    IT_PANIC("unexpected extra data after " << curr_token->to_string());
                }
            }
        }


        T value(const Context c) {
            context = c;
            return eval();
        }

        private:
        Context context;
        token_iterator curr_token;

        // advance the token iterator to the next token to be evaluated
        void get_token() { ++curr_token; }

        token_iterator last_token() {
            return curr_token - 1;
        }

        token_value next_token() const { return *(curr_token + 1); }

        // handle primaries, number, name, parenthesis (which evals() it's contents)
        T prim() {
            get_token();
            auto t = *curr_token;

            switch (curr_token->id) {

                case NUMBER: {
                    T v = curr_token->number;
                    get_token();
                    return v;
                }

                case NAME: {   
                    get_token(); // get the next token
                    switch (curr_token->id) {
                        case LP: { // function
                            std::vector<param_type> params;
                            eval_list(params);
                            if (*curr_token != RP) IT_PANIC(") expected");
                            get_token(); // eat ')'
                            return eval_function(t.name, context, params);
                        }
                        case DOT:{ // name.name notation
                            get_token();
                            if (curr_token->id != NAME) IT_PANIC("expected name after '.'");
                            auto s = *curr_token;
                            get_token();
                            return eval_variable_list(t.name, s.name, context);
                        }
                        default: // just a regular variable
                            return eval_variable(t.name, context);
                    }
                }
                case NOT:
                    return !prim();
                case MINUS: // unary minus
                    return -prim();
                case LP: {
                    T e = eval();
                    if (*curr_token != RP) IT_PANIC(") expected");
                    get_token(); // eat ')'
                    return e;
                }
                default:
                    IT_PANIC("primary expected, got " << curr_token->to_string());
            }
        }

        // multiply and divide
        T term() {
            T left = prim();

            for (;;) {
                switch (curr_token->id) {
                    case MUL: 
                        left *= prim(); 
                        break;
                    case POW: 
                        left = (T) std::pow(left, prim());
                        break;
                    case DIV:
                        if (T d = prim()) {
                            left /= d;
                            break;
                        }
                        IT_PANIC("divide by 0");
                    case REM: 
                        left %= prim(); 
                        break;
                    default: return left;
                }
            }
        }

        // add and subtract
        T plus_minus() {
            T left = term();

            for (;;)
                switch (curr_token->id) {
                case PLUS: left += term(); break;
                case MINUS: left -= term(); break;
                default: return left;
                }
        }

        // bitwise shift left and right
        T bitwise_shift() {
            T left = plus_minus();
            for (;;) 
                switch (curr_token->id) {
                case BIT_SHIFT_LEFT: left <<= plus_minus(); break;
                case BIT_SHIFT_RIGHT: left >>= plus_minus(); break;
                default: return left;
                }
        }

        // relational less and greater
        T relational() {
            T left = bitwise_shift();
            for (;;) 
                switch (curr_token->id) {
                case LESS: (left = left < bitwise_shift()); break;
                case LESS_EQUAL: left = (left <= bitwise_shift()); break;
                case GREATER: left = (left > bitwise_shift()); break;
                case GREATER_EQUAL: left = (left >= bitwise_shift()); break;
                default: return left;
                }
        }
        
        // relational equal/not equal
        T relational_equal() {
            T left = relational();
            for (;;) 
                switch (curr_token->id) {
                case EQUAL: left = left == relational(); break;
                case NOT_EQUAL: left = left != relational(); break;
                default: return left;
                }
        }

        // bitwise &
        T bitwise_and() {
            T left = relational_equal();
            for (;;) 
                switch (curr_token->id) {
                case BIT_WISE_AND: left = left & relational_equal(); break;
                case BIT_WISE_OR: left = left | relational_equal(); break;
                default: return left;
                }
        }

        // logical &&
        T logical_and() {
            T left = bitwise_and();
            for (;;) 
                switch (curr_token->id) {
                    // order matters in case of short circuit
                    case LOGICAL_AND: left = bitwise_and() && left; break;
                    default: return left;
                }
        }
        // logical ||
        T logical_or() {
            T left = logical_and();
            for (;;) 
                switch (curr_token->id) {
                    case LOGICAL_OR: left = logical_and() || left; break;
                    default: return left;
                }
        }

        T terniary() {
            T left = logical_or();
            for (;;) 
                switch (curr_token->id) {
                    case QUESTION: {
                        T a = eval();
                        if (*curr_token != COLON) IT_PANIC("':' expected, got " << curr_token->to_string());
                        T b = eval();
                        return left ? a : b;
                    }
                    default: return left;
                }
        }

        T eval() {
            return terniary();
        }

        param_type get_function_param() {
            param_type p;
            if (next_token() == NAME_REF) {
                // a quoted name
                get_token();
                p.name = curr_token->name;
                get_token();
            } else {
                p.number = eval();
            }
            return p;
        }

        void eval_list(std::vector<param_type> & params) {
            if (*(curr_token + 1) == RP) {
                get_token();
                return;
            }
            params.push_back(get_function_param());
            for (;;) 
                switch (curr_token->id) {
                    case COMMA: 
                        params.push_back(get_function_param());
                        break;
                    default: return;
                }
        }

    };


    private:
    std::istringstream input; // pointer to input stream
    std::string initial_string;
    std::vector<token_value> tokens;

    public:
    expr_type(T value) { // set to constant
        tokens.push_back(START);
        tokens.push_back(token_value(NUMBER, value));
        tokens.push_back(END);
    }

    std::string to_string() const { return initial_string; }

    // we denote empty as constant max value for T
    expr_type() : expr_type(std::numeric_limits<T>::max()) {
    }

    expr_type(const std::string & expr) : input(expr), initial_string(expr) {
        tokenize();
        if (tokens.empty()) IT_PANIC("no tokens in expression");
    }

    template <typename Context>
    expr_type(const std::string & expr, Context c) : expr_type(expr) {
        compile(c);
    }

    expr_type(const expr_type & b) {
        tokens = b.tokens;
    }

    bool empty() const { 
        return (tokens.size() == 3) && tokens[1] == token_value(NUMBER, std::numeric_limits<T>::max());
    }

    expr_type& operator=(const expr_type& b) {
        tokens = b.tokens;
        return *this;
    }

    bool more() const {
        return static_cast<bool>(input);
    }

    template <typename Context>
    T value(Context c) const {
        parser<Context> p(tokens.begin());
        return p.value(c);
    }

    template <typename Context>
    void compile(Context c) const {
        parser<Context> p(tokens.begin());
        p.compile(c);
    }

    private:

    bool advance(char & ch) {
        return static_cast<bool>(input.get(ch));
    }

    void unget() {
        input.unget();
    }

    token_value advance_check(token_id first, char c, token_id second) {
        char ch;
        advance(ch);
        if (ch == c) return second;
        unget();
        return first;
    }

    token_value advance(char c, token_id expected) {
        char ch;
        advance(ch);
        if (ch == c) return expected;
        IT_PANIC("expected " << c << " got " << ch);
    }

    token_value read_hex_number() {
        auto tok = read_name();
        auto b = bitstring(16, tok.name.c_str());
        if (b.empty()) IT_PANIC("invalid hex string");
        return token_value(NUMBER, to_integer<T>(b));
    }

    token_value read_bin_number() {
        auto tok = read_name();
        auto b = bitstring(2, tok.name.c_str());
        if (b.empty()) IT_PANIC("invalid bin string");
        return token_value(NUMBER, to_integer<T>(b));
    }

    token_value read_number() {
        token_value tok(NUMBER);
        input >> tok.number;
        return tok;
    }

    // read a string of alphunumerics
    token_value read_name() {
        char ch;
        std::string string_value;
        while (advance(ch) && (isalnum(ch) || (ch == '_')))
            string_value.push_back(ch);
        unget();
        if (string_value == "ls") return BIT_SHIFT_LEFT;
        if (string_value == "rs") return BIT_SHIFT_RIGHT;
        if (string_value == "eq") return EQUAL;
        if (string_value == "lt") return LESS;
        if (string_value == "gt") return GREATER;
        if (string_value == "lte") return LESS_EQUAL;
        if (string_value == "gte") return GREATER_EQUAL;
        if (string_value == "or") return LOGICAL_OR;
        if (string_value == "and") return LOGICAL_AND;
        token_value t(NAME, string_value);
        return t;
    }

    token_value read_quoted_name(char end_ch, token_id id) {
        token_value t(id);
        char ch;
        while (advance(ch) && ch != end_ch) {
            t.name.push_back(ch);
        }
        if (ch != end_ch) IT_PANIC("no closing '" << end_ch << "' found in " << t.to_string());
        return t;
    }

    std::string read_remaining() {
        char ch;
        std::string s;
        while (advance(ch)) s.push_back(ch);
        return s;
    }

    token_value get_token_from_input() {
        char ch;

        do {    // skip whitespace except '\en'
            if(!advance(ch)) return END;
        } while (isspace(ch));

        switch (ch) {
        case '*':
            return advance_check(MUL, '*', POW);
        case '/':
            return DIV;
        case '%':
            return REM;
        case '&':
            return advance_check(BIT_WISE_AND, '&', LOGICAL_AND);
        case '+':
            return PLUS;
        case '-':
            return MINUS;
        case '(':
            return LP;
        case ')':
            return RP;
        case '}':
            return RC;
        case '?':
            return QUESTION;
        case ':':
            return COLON;
        case ',':
            return COMMA;
        case '=':
            return advance('=', EQUAL);
        case '!':
            return advance_check(NOT, '=', NOT_EQUAL);
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            unget();
            return read_number();
        case '.':
            return DOT;
        case '#':
            return read_hex_number();
        case '@':
            return read_bin_number();
        case '<':
            advance(ch);
            switch (ch) {
                case '<' :
                    return BIT_SHIFT_LEFT;
                case '=' :
                    return LESS_EQUAL;
                default:
                    unget();
                    return LESS;
            }
        case '>':
            advance(ch);
            switch (ch) {
                case '>' :
                    return BIT_SHIFT_RIGHT;
                case '=' :
                    return GREATER_EQUAL;
                default:
                    unget();
                    return GREATER;
            }
        case '|':
            return advance_check(BIT_WISE_OR, '|', LOGICAL_OR);

        case '\'' : // '{', denoting a quoted name     
            return read_quoted_name('\'', NAME_REF);
        case '{' : // '{', denoting a quoted name     
            return read_quoted_name('}', NAME);

        default:            // NAME or FUNCTION
            if (isalpha(ch) || ch == '_') {
                unget();
                return read_name();
            }
            IT_PANIC("bad token: \'" << ch << "\'");
        }
    }

    void tokenize() {
        tokens.push_back(START);
        while (more()) {
            tokens.push_back(get_token_from_input());
        }
        if (tokens.back() != END) tokens.push_back(END);
    }

};

typedef expr_type<int64_t> expr;
typedef expr_type<int64_t> expression;

template <typename Stream, typename T>
Stream & operator<<(Stream & os, expr_type<T> x) {
    os << x.to_string();
    return os;
}

} // namespace
