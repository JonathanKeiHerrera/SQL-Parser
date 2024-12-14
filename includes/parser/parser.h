#pragma once

#include "../../includes/table/table.h"
#include "../../includes/tokenizer/token.h"
#include "../../includes/tokenizer/stokenize.h"
#include "../../includes/tokenizer/state_machines_function.h"
#include "../../includes/tokenizer/constants.h"

#include <string>
#include <vector>
#include <cassert>

using namespace std;

class Parser {
public:
    Parser();
    Parser(char* s);

    void set_string(char* s);
    int get_column(string token);
    bool get_parse_tree(Queue<Token*> input_queue);
    MMap<string, string> parse_tree();

    void build_keyword_list();
    void make_table();
    bool fail();

private:
    MMap<string, string> ptree;
    Map<string, long> keywords;
    Queue<Token*> input_queue;
    bool fail_flag;
    char internal_buffer[MAX_BUFFER];

    int adj_table[MAX_ROWS][MAX_COLUMNS];

    enum keys_columns {
        ZERO, SELECT, FROM, STAR, WHERE, CONDITION, SYMBOL,
        CREATE, MAKE, TABLE, FIELDS,
        INSERT, INTO, VALUES,
        LOG, COMMA, PAREN, OP
    };
};

// DEFAULT CONSTRUCTOR
Parser::Parser() {
    fail_flag = false;
    internal_buffer[0] = '\0';
    make_table();
    build_keyword_list();
}

// CONSTRUCTOR: 
//  ARGUMENTS: s -- The command that will be run
//  Calls make_table, build_keyword_list, for setting up the parser
//  Parses the command through set_string
Parser::Parser(char* s) {
    fail_flag = false;
    // copy str to internal buffer
    int i = 0;
    for (; s[i] != '\0'; i++) {
        internal_buffer[i] = s[i];
    }
    internal_buffer[i] = '\0';

    // initialize "make" adjacency table
    make_table();

    // build keyword list
    build_keyword_list();

    // set string
    set_string(s);
}

// FUNCTION: set_string()
//  ARGUMENTS: s -- The command that is being parsed
//  Parses a command by tokenizing it and making sure it is valid through:
//   1) Must have closed quotes
//   2) Must have in order parenthesis and must be closed parenthesis
//   3) More error checking inside of get_parse_tree()    
void Parser::set_string(char* s) {
    // clear the input queue (idk prob already is?)
    while (!input_queue.empty()) {
        input_queue.pop();
    }

    // Tokenize input buffer into input_q
    STokenizer stk(s);
    bool quote_flag = false;
    vector<bool> paren_flags;
    string hold = "";

    Token* current_token = nullptr;
    current_token = new Token();
    stk >> *current_token;

    while (stk.more()) {

        if (current_token->type() == TOKEN_LPAREN) {
            paren_flags.push_back(true);
        }
        if (current_token->type() == TOKEN_RPAREN) {
            if (paren_flags.empty()) {
                cout << "ERROR: OUT OF ORDER PARENTHESIS" << endl;
                fail_flag = true;
                return;
            }

            paren_flags.pop_back();
        }

        if (current_token->type() == TOKEN_QUOTE) {
            // IF FOUND A QUOTE:
            // Either set the quote or flag or unset it.
            // if quote flag is already set, create a new token using the combined strings
            if (!quote_flag) {
                quote_flag = true;
            }
            else {
                current_token = new Token(hold, 2);
                input_queue.push(current_token);
                quote_flag = false;
                hold.clear();
            }
        }
        else {
            // IF NOT A QUOTE:
            // Push the token into the queue, unless...
            // If the quote flag is set, then instead of pushing in queue, attach at end of a string
            if (quote_flag) {
                hold += current_token->token_str();
            }
            else if (current_token->type_string() != "SPACE") {
                input_queue.push(current_token);
            }
        }

        current_token = new Token();
        stk >> *current_token;
    }

    if (quote_flag) {
        cout << "ERROR: UNCLOSED QUOTES" << endl;
        fail_flag = true;
        return;
    }

    if (!paren_flags.empty()) {
        cout << "ERROR: UNCLOSED PARENTHESIS" << endl;
        fail_flag = true;
        return;
    }

    bool is_success = get_parse_tree(input_queue);
    fail_flag = !is_success;

    return;
}

// FUNCTION: get_column()
//  ARGUMENTS: token -- a string used to look for a corresponding integer from the keywords list
//  Uses the keywords list to retrieve an integer. The integer is used for the columns of the adj table
int Parser::get_column(string token) {
    Map<string, long>::Iterator it = keywords.find(token);

    // Return either a value from keywords, or default to SYMBOL enum if not found.
    if (keywords.contains(Pair<string, long>(token))) {
        return keywords[token];
    }
    else {
        return SYMBOL;   // DEFAULT TO SYMBOL
    }
}

// FUNCTION: get_parse_tree()
//  ARGUMENTS: queue -- A queue of token pointers, from the command
//  Pops each token from the queue, then similarly to the state machine, goes to different states based on the token contents
//  Also includes error checking for going into an invalid state.
bool Parser::get_parse_tree(Queue<Token*> queue) {
    ptree.clear();

    // have to create a new mmap, otherwise program crashes...
    // Dude I don't know why 
    MMap<string, string> temp;

    Token* t;
    long column;
    int state = 0;

    while (!queue.empty()) {
        if (state == -1) {
            cout << "ERROR: WENT TO INVALID STATE" << endl;
            return false;
        }

        t = queue.pop();

        string lower = "";
        for (char& x : t->token_str()) {
            lower += tolower(x);
        }

        column = get_column(lower);
        state = adj_table[state][column];
        string yes = "yes";

        switch (state) {
            // COMMANDS: SELECT, MAKE, CREATE, INSERT
        case 1:
        case 20:
        case 30:
            temp["command"] += t->token_str();
            break;

            // FROM: does not matter
        case 5:
            break;

            // TABLE_NAME: 
        case 6:
        case 22:
        case 32:
            temp["table_name"] += t->token_str();
            break;

            // FIELDS: 
        case 2:
        case 4:
            temp["fields"] += t->token_str();
            break;

            // CONDITION:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
            if (t->token_str() == "where") {
                temp["where"] += yes;
            }
            else {
                temp["condition"] += t->token_str();
            }
            break;

            // VALUES:
        case 24:
            temp["values"] += t->token_str();
            break;

            // COLS:
        case 34:
            temp["cols"] += t->token_str();
            break;

        default:
            break;
        }

    }

    ptree = temp;

    // Check for success state
    if (adj_table[state][0] == 1) {
        return true;
    }
    else {
        ptree.clear();
        return false;
    }
}

MMap<string, string> Parser::parse_tree() {
    return ptree;
}

void Parser::build_keyword_list() {
    // Builds the keyword map used to map keyword strings to adjacency matrix column numbers
    keywords["select"] = SELECT;
    keywords["from"] = FROM;
    keywords["*"] = STAR;
    keywords["where"] = WHERE;
    keywords["create"] = CREATE;
    keywords["make"] = MAKE;
    keywords["table"] = TABLE;
    keywords["fields"] = FIELDS;
    keywords["insert"] = INSERT;
    keywords["into"] = INTO;
    keywords["values"] = VALUES;
    keywords["and"] = LOG;
    keywords["or"] = LOG;
    keywords[","] = COMMA;
    keywords["("] = PAREN;
    keywords[")"] = PAREN;
    keywords["="] = OP;
    keywords["<="] = OP;
    keywords["<"] = OP;
    keywords[">="] = OP;
    keywords[">"] = OP;
}


// FUNCTION: make_table()
//  ARGUMENTS: NONE
//  Creates a state machine table for the different strings in the commmand
//  NOTE: Since the parser checks for a 1 to determine a success state, there is no need to mark a fail state with 0.
void Parser::make_table() {
    // Table for SQL keywords or something

    init_table(adj_table);
    //   0     1       2    3      4       5         6       7      8       9          10      11     12     13     14  15    16     17   18     19
    // ZERO, SELECT, FROM, STAR, WHERE, CONDITION, SYMBOL, CREATE, MAKE, TABLE_NAME, FIELDS, INSERT, INTO, VALUES, AND, OR, COMMA, LPAREN, OP, RPAREN

    // SELECT -> SYMBOL(fields) -> FROM -> SYMBOL(table_name)
    mark_cell(0, adj_table, SELECT, 1);

    // SELECT -> SYMBOL(fields) -> FROM
    mark_cell(1, adj_table, SYMBOL, 2);
    mark_cell(2, adj_table, COMMA, 3);
    mark_cell(3, adj_table, SYMBOL, 2);
    mark_cell(2, adj_table, FROM, 5);

    // SELECT -> STAR -> FROM
    mark_cell(1, adj_table, STAR, 4);
    mark_cell(4, adj_table, FROM, 5);

    // FROM -> SYMBOL(table_name) 
    mark_cell(5, adj_table, SYMBOL, 6);
    mark_success(adj_table, 6);

    // SYMBOL(table_name) -> WHERE -> SYMBOL(condition) 
    mark_cell(6, adj_table, WHERE, 7);

    // SYMBOL -> OP -> SYMBOL 
    mark_cell(7, adj_table, SYMBOL, 8);
    mark_cell(8, adj_table, OP, 9);
    mark_cell(9, adj_table, SYMBOL, 10);
    mark_success(adj_table, 10);

    // LOG -> LOOP -> LOG
    mark_cell(10, adj_table, LOG, 11);
    mark_cell(14, adj_table, LOG, 11);

    // SOS LOOP:
    mark_cell(11, adj_table, SYMBOL, 12);
    mark_cell(12, adj_table, OP, 13);
    mark_cell(13, adj_table, SYMBOL, 14);
    mark_success(adj_table, 14);

    // ACCEPT PARENTHESIS BEFORE/AFTER SOS
    mark_cell(7, adj_table, PAREN, 7);
    mark_cell(10, adj_table, PAREN, 10);
    mark_cell(11, adj_table, PAREN, 11);
    mark_cell(14, adj_table, PAREN, 14);

    // INSERT -> INTO -> SYMBOL (table_name) -> VALUES -> SYMBOL (fields)
    mark_cell(0, adj_table, INSERT, 20);
    mark_cell(20, adj_table, INTO, 21);
    mark_cell(21, adj_table, SYMBOL, 22);
    mark_cell(22, adj_table, VALUES, 23);
    mark_cell(23, adj_table, SYMBOL, 24);
    mark_cell(24, adj_table, COMMA, 25);
    mark_cell(25, adj_table, SYMBOL, 24);
    mark_success(adj_table, 24);


    // MAKE/CREATE -> TABLE -> SYMBOL (table_name) -> FIELDS -> SYMBOL (fields)
    mark_cell(0, adj_table, CREATE, 30);
    mark_cell(0, adj_table, MAKE, 30);
    mark_cell(30, adj_table, TABLE, 31);
    mark_cell(31, adj_table, SYMBOL, 32);
    mark_cell(32, adj_table, FIELDS, 33);
    mark_cell(33, adj_table, SYMBOL, 34);
    mark_cell(34, adj_table, COMMA, 35);
    mark_cell(35, adj_table, SYMBOL, 34);
    mark_success(adj_table, 34);

}

bool Parser::fail() {
    return fail_flag;
}