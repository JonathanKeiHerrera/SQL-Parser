#pragma once

const int MAX_COLUMNS = 256;
const int MAX_ROWS = 100;
const int MAX_BUFFER = 200;

const char ALFA[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char DIGITS[] = "0123456789";
const char OPERATORS[] = "><=!+-%&|";
const char SPACES[] = { ' ', '\t', '\n', '\0' };
const char PUNC[] = "?.,:;'`~!";
const char QUOTE[] = "\"";
const char LPAREN[] = "(";
const char RPAREN[] = ")";

const int START_DOUBLE = 0;
const int START_SPACES = 4;
const int START_ALPHA = 6;
const int START_OPERATOR = 20;
const int START_PUNC = 10;

//token types:
const int TOKEN_NUMBER = 1;
const int TOKEN_ALPHA = 2;
const int TOKEN_SPACE = 3;
const int TOKEN_OPERATOR = 4;
const int TOKEN_PUNC = 5;
const int TOKEN_QUOTE = 9;
const int TOKEN_LPAREN = 11;
const int TOKEN_RPAREN = 12;

const int TOKEN_UNKNOWN = -1;
const int TOKEN_END = -2;

enum TOKEN_TYPES {
    LEFT_PAR,
    RIGHT_PAR,
    OR,
    AND,
    LOGICAL,
    RELATIONAL,
    TOKEN_STR,
    RESULT_SET,
    OPERATOR
};