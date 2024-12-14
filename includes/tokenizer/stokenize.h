#pragma once

#include "token.h"
#include "constants.h"
#include "state_machines_function.h"

#include <iostream>
#include <string>
#include <cstring>

using namespace std;


class STokenizer {
public:
    STokenizer();
    STokenizer(char str[]);
    bool done();
    bool more();
    friend STokenizer& operator >> (STokenizer& s, Token& t);

    void set_string(char str[]);

private:
    void make_table(int table[][MAX_COLUMNS]);
    bool get_token(int& start_state, string& token);

    char _buffer[MAX_BUFFER];
    int _pos;
    bool _done;
    static int _table[MAX_ROWS][MAX_COLUMNS];
};

int STokenizer::_table[MAX_ROWS][MAX_COLUMNS];

STokenizer::STokenizer() {
    _buffer[0] = '\0';
    _done = false;
    _pos = 0;
    make_table(_table);
}

STokenizer::STokenizer(char str[]) {
    int i = 0;
    for (; str[i] != '\0'; i++) {
        _buffer[i] = str[i];
    }
    _buffer[i] = '\0';
    _pos = 0;
    _done = false;
    make_table(_table);
}

bool STokenizer::done() {

    return _done;

}

bool STokenizer::more() {
    return !done();
}

void STokenizer::set_string(char str[]) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        _buffer[i] = str[i];
    }

    _buffer[i] = '\0';
    _done = false;
    _pos = 0;
}

void STokenizer::make_table(int table[][MAX_COLUMNS]) {
    // initializes table with -1
    init_table(table);

    // Success states: 
        // 1 - Numbers 
        // 2 - Alphabet
        // 3 - Spaces
        // 4 - Operators
        // 5 - Punctuation
        // 7 - Double (Complete)
        // 9 - Quote 
    mark_success(table, TOKEN_NUMBER);
    mark_success(table, TOKEN_ALPHA);
    mark_success(table, TOKEN_SPACE);
    mark_success(table, TOKEN_OPERATOR);
    mark_success(table, 7);
    mark_success(table, TOKEN_QUOTE);
    mark_success(table, TOKEN_LPAREN);
    mark_success(table, TOKEN_RPAREN);


    // Fail State:
        // 5 - Punctuation
        // 6 - Double (Incomplete)
    mark_fail(table, TOKEN_PUNC);
    mark_fail(table, 6);
    // mark_fail(table, TOKEN_BEG_QUOTE);


    // ROW 0 -- Default state -> First Character decides next state
    mark_cells(0, table, DIGITS, TOKEN_NUMBER);
    mark_cells(0, table, ALFA, TOKEN_ALPHA);
    mark_cells(0, table, SPACES, TOKEN_SPACE);
    mark_cells(0, table, OPERATORS, TOKEN_OPERATOR);
    mark_cells(0, table, PUNC, TOKEN_PUNC);
    mark_cells(0, table, QUOTE, TOKEN_QUOTE);
    mark_cells(0, table, LPAREN, TOKEN_LPAREN);
    mark_cells(0, table, RPAREN, TOKEN_RPAREN);

    // IF THE CHARACTER IS A NULL CHARACTER...
    mark_cell(0, table, '\0', TOKEN_END);

    // ROW 1 -- NUMBER -> Numbers or Punctuation allowed...
    // Cells for numbers = TOKEN_NUMBER, . = 6, else = -1 
    mark_cells(TOKEN_NUMBER, table, DIGITS, TOKEN_NUMBER);
    mark_cells(TOKEN_NUMBER, table, '.', '.', 6);

    // ROW 2 -- ALPHA -> Only Letters allowed after
    // Cells for letters = 1, non-leltters = 0
    mark_cells(TOKEN_ALPHA, table, ALFA, TOKEN_ALPHA);

    // ROW 3 -- SPACE -> Only Spaces allowed after
    // Cells for Spaces = 1, non-spaces = 0
    mark_cells(TOKEN_SPACE, table, SPACES, TOKEN_SPACE);

    // ROW 4 -- OPERATOR -> Only Operators allowed after
    // Cells for operators = 1, non-operators = 0
    mark_cells(TOKEN_OPERATOR, table, OPERATORS, TOKEN_OPERATOR);

    // ROW 5 -- PUNC -> Fail state, nothing allowed after

    // ROW 6 -- Double (Incomplete) -> MUST HAVE A NUMBER AFTER
    mark_cells(6, table, DIGITS, 7);

    // ROW 7 -- Double (Complete) -> Only Numbers allowed after
    mark_cells(7, table, DIGITS, 7);

    // ROW 9 -- Quote (Complete) -> no need to read anymore

}

bool STokenizer::get_token(int& start_state, string& token) {
    int last_success_pos = _pos;
    int temp_pos = _pos;
    int temp_state = start_state;

    size_t buffer_length = strlen(_buffer);

    if (_pos >= buffer_length) {
        _done = true;
        return false;
    }

    // MAIN LOOP
    while (_buffer[temp_pos] != '\0') {

        // Checks if char is valid
        if (_buffer[temp_pos] < 0 || _buffer[temp_pos] >= MAX_COLUMNS) {
            break;
        }

        // Finds next state, checks if valid
        int next_state = _table[temp_state][_buffer[temp_pos]];
        if (next_state < 0 || next_state > MAX_ROWS) {
            break;
        }

        // Updates the token, state, and position
        temp_state = next_state;
        if (is_success(_table, temp_state)) {
            last_success_pos = temp_pos;
            start_state = temp_state;
        }
        temp_pos++;
    }

    // Updates Token:
    for (int i = _pos; i <= last_success_pos; i++) {
        token += _buffer[i];
        if (!is_success(_table, start_state) && !is_success(_table, temp_state)) {
            start_state = temp_state;
        }
    }

    _pos = last_success_pos + 1;
    return (is_success(_table, start_state));
}

STokenizer& operator >> (STokenizer& stk, Token& t) {
    // gets next token
    int state = 0;
    int type = 0;
    string token = "";

    stk.get_token(state, token);

    switch (state) {
    case TOKEN_NUMBER:
        type = TOKEN_NUMBER;
        break;
    case TOKEN_ALPHA:
        type = TOKEN_ALPHA;
        break;
    case TOKEN_SPACE:
        type = TOKEN_SPACE;
        break;
    case TOKEN_OPERATOR:
        type = TOKEN_OPERATOR;
        break;
    case TOKEN_PUNC:
        type = TOKEN_PUNC;
        break;
    case 7:
        type = TOKEN_NUMBER;
        break;
    case TOKEN_QUOTE:
        type = TOKEN_QUOTE;
        break;
    case TOKEN_LPAREN:
        type = TOKEN_LPAREN;
        break;
    case TOKEN_RPAREN:
        type = TOKEN_RPAREN;
        break;
    default:
        type = TOKEN_UNKNOWN;
    }

    t = Token(token, type);
    return stk;
}
