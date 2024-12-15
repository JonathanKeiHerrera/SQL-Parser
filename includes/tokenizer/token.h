#ifndef TOKEN_H
#define TOKEN_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "constants.h"

using namespace std;

class Token {
public:
    Token();
    Token(string str, int type);
    friend ostream& operator <<(ostream& outs, const Token& t);
    int type() const;
    string type_string() const;
    string token_str() const;
    virtual TOKEN_TYPES type_of();
private:
    string _token;
    int _type;
};

Token::Token() : _token(""), _type(0) {}

Token::Token(string str, int type) : _token(str), _type(type) {}

ostream& operator <<(ostream& outs, const Token& t) {
    outs << "|" << t._token << "|";
    return outs;
}

int Token::type() const {
    return _type;
}

string Token::type_string() const {
    switch (_type) {
    case 1: return "NUMBER";
    case 2: return "ALPHA";
    case 3: return "SPACE";
    case 4: return "OPERATOR";
    case 5: return "PUNC";
    case 9: return "QUOTE";
    case 11: return "LPAREN";
    case 12: return "RPAREN";
    default: return "UNKNOWN";
    }
}

string Token::token_str() const { return _token; }
TOKEN_TYPES Token::type_of() { return TOKEN_STR; }

class TokenStr : public Token {
public:
    TokenStr(string token) : Token(token, TOKEN_STR) {}
    TOKEN_TYPES type_of() { return TOKEN_STR; }
};

class ResultSet : public Token {
public:
    ResultSet() : Token("result", RESULT_SET) {}
    ResultSet(string name) : Token(name, RESULT_SET) {}
    ResultSet(const ResultSet& other) : Token("result", RESULT_SET) { records = other.get_records(); }
    TOKEN_TYPES type_of() { return RESULT_SET; }
    ResultSet* merge(const ResultSet* x, const ResultSet* y);
    ResultSet* intersection(const ResultSet* x, const ResultSet* y);

    vector<long> get_records() const;
    void set_records(const vector<long> vec);

private:
    vector<long> records;
};

vector<long> ResultSet::get_records() const {
    return records;
}

void ResultSet::set_records(const vector<long> vec) {
    records.clear();
    records = vec;
}

ResultSet* ResultSet::merge(const ResultSet* x, const ResultSet* y) {
    vector<long> one = x->get_records();
    vector<long> two = y->get_records();

    sort(one.begin(), one.end());
    sort(two.begin(), two.end());

    vector<long> final;

    int count_one = 0;
    int count_two = 0;

    while (count_one < one.size() && count_two < two.size()) {
        if (one[count_one] < two[count_two]) {
            if (final.empty() || final.back() != one[count_one]) {
                final.push_back(one[count_one]);
            }
            count_one++;
        }
        else if (one[count_one] > two[count_two]) {
            if (final.empty() || final.back() != two[count_two]) {
                final.push_back(two[count_two]);
            }
            count_two++;
        }
        else {
            if (final.empty() || final.back() != one[count_one]) {
                final.push_back(one[count_one]);
            }
            count_one++;
            count_two++;
        }
    }

    while (count_one < one.size()) {
        if (final.empty() || final.back() != one[count_one]) {
            final.push_back(one[count_one]);
        }
        count_one++;
    }
    while (count_two < two.size()) {
        if (final.empty() || final.back() != two[count_two]) {
            final.push_back(two[count_two]);
        }
        count_two++;
    }

    ResultSet* result = new ResultSet;
    result->set_records(final);
    return result;
}

ResultSet* ResultSet::intersection(const ResultSet* x, const ResultSet* y) {
    vector<long> one = x->get_records();
    vector<long> two = y->get_records();
    vector<long> vec;

    sort(one.begin(), one.end());
    sort(two.begin(), two.end());

    int count_one = 0;
    int count_two = 0;

    while (count_one < one.size() && count_two < two.size()) {
        if (one[count_one] == two[count_two]) {
            vec.push_back(one[count_one]);
            count_one++;
            count_two++;
        }
        else if (one[count_one] < two[count_two]) {
            count_one++;
        }
        else {
            count_two++;
        }
    }

    ResultSet* result = new ResultSet;
    result->set_records(vec);
    return result;
}


class Operator : public Token {
public:
    Operator(string op) : Token(op, OPERATOR) {}
    Operator(string op, TOKEN_TYPES type) : Token(op, type) {}
    TOKEN_TYPES type_of() {
        return OPERATOR;
    }

};


class Relational : public Operator {
public:
    Relational(string relational) : Operator(relational, RELATIONAL) {}
    TOKEN_TYPES type_of() { return RELATIONAL; }

    ResultSet* eval(MMap<string, long> mmap, Token* left, Token* right);
};

ResultSet* Relational::eval(MMap<string, long> mmap, Token* left, Token* right) {
    ResultSet* result = new ResultSet;

    MMap<string, long>::Iterator it = mmap.begin();
    MMap<string, long>::Iterator end = mmap.end();

    if (token_str() == "=") {
        result->set_records(mmap[right->token_str()]);
        return result;
    }
    else if (token_str() == ">=") {
        it = mmap.lower_bound(right->token_str());

    }
    else if (token_str() == ">") {
        it = mmap.upper_bound(right->token_str());

    }
    else if (token_str() == "<=") {
        end = mmap.upper_bound(right->token_str());

    }
    else if (token_str() == "<") {
        end = mmap.lower_bound(right->token_str());

    }

    vector<long> recnos;
    for (; it != end; it++) {
        vector<long> next = (*it).value_list;
        for (long x : next) {
            recnos.push_back(x);
        }
    }

    result->set_records(recnos);
    return result;
}

class Logical : public Operator {
public:
    Logical(string op) : Operator(op, LOGICAL) { op_str = op; }
    TOKEN_TYPES type_of() {
        if (op_str == "and") {
            return AND;
        }
        else if (op_str == "or") {
            return OR;
        }
        return LOGICAL;
    }
    ResultSet* eval(Token* left, Token* right);

private:
    string op_str;
};

ResultSet* Logical::eval(Token* left, Token* right) {
    ResultSet* result = new ResultSet;

    if (token_str() == "and") {
        result = result->intersection(static_cast<ResultSet*>(left), static_cast<ResultSet*>(right));
    }
    else if (token_str() == "or") {
        result = result->merge(static_cast<ResultSet*>(left), static_cast<ResultSet*>(right));
    }
    return result;
}

class LeftPar : public Operator {
public:
    LeftPar() : Operator("(", LEFT_PAR) {}
    TOKEN_TYPES type_of() { return LEFT_PAR; }
};

class RightPar : public Operator {
public:
    RightPar() : Operator(")", RIGHT_PAR) {}
    TOKEN_TYPES type_of() { return RIGHT_PAR; }
};
#endif