#pragma once

#include "../../includes/parser/parser.h"
#include "../../includes/table/table.h"
#include "../../includes/binary_files/utilities.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>

using namespace std;

class SQL {
public:
    SQL();
    SQL(string file_name);

    Table command(string command);
    vector<long> select_recnos() { return t.select_recnos(); }

    Table select();
    Table create_make();
    Table insert();

    friend ostream& operator << (ostream& outs, SQL sequel) {
        return outs;
    }

private:
    Parser p;
    Table t;
    vector<string> table_names;
    vector<long> _select_recnos;
};

// DEFAULT CONSTRUCTOR:
//  NOT USING
SQL::SQL() {}

// MAIN CONSTRUCTOR:
//  ARGUMENTS: file_name - Name of batch file to run commands from
//  Continuously calls command function to run commands from each line
SQL::SQL(string file_name) {

    fstream f;
    open_fileRW_text(f, file_name.c_str());
    string str = "";

    cout << "> " << file_name << endl;


    f.seekg(0, std::ios::beg);
    if (f.peek() == EOF) {
        cout << "File is empty!" << endl;
    }

    while (getline(f, str)) {
        cout << "Reading line: " << str << endl;
        if (str.substr(0, 2) == "//") {
            cout << str.substr(2) << endl;
        }
        else if (str == "") {
            cout << endl;
        }
        else {
            cout << "COMMAND: " << str << endl;
            t = command(str);
        }
    }

    for (string table : table_names) {
        remove(table.c_str());
    }
    remove("table.bin");

    f.close();
}

// FUNCTION: command:
//  ARGUMENTS: command - a string input which is taken from the constructor
Table SQL::command(string command) {
    char s[300];
    int i = 0;
    for (; i < command.size(); i++) {
        s[i] = command[i];
    }
    s[i] = '\0';

    // Parse a command:
    p.set_string(s);

    if (p.fail()) {
        return t;
    }

    vector<string> instruction = p.parse_tree()["command"];

    // Make a table from said command:
    if (instruction[0] == "select") {
        t = select();
        cout << t << endl;

    }
    else if (instruction[0] == "create" || instruction[0] == "make") {
        t = create_make();
    }
    else if (instruction[0] == "insert") {
        t = insert();
    }

    table_names.push_back(t.get_file());
    table_names.push_back(t.get_field_file());

    // Return the table
    return t;
}


// FUNCTION: select()
//  ARGUMENTS: NONE
//  Is called from command if:
//   1) SQL has received a valid command
//   2) The ptree map contains the value "select" under the key "command" 
Table SQL::select() {
    vector<string> table_name = p.parse_tree()["table_name"];
    vector<string> fields = p.parse_tree()["fields"];
    vector<string> condition = p.parse_tree()["condition"];

    t = Table(table_name[0]);

    if (fields[0] == "*") {
        if (condition.empty()) {
            return t.select_all();
        }
        else {
            return t.select(t.get_fields(), condition);
        }
    }
    if (condition.empty()) {
        return t.select_all(fields);
    }
    return t.select(fields, condition);
}

// FUNCTION: create_make()
//  ARGUMENTS: NONE
//  Creates a table given a valid commmand with a table name and its columns
Table SQL::create_make() {
    vector<string> table_name = p.parse_tree()["table_name"];
    vector<string> fields = p.parse_tree()["cols"];

    t = Table(table_name[0], fields);
    return t;
}

// FUNCTION: insert()
//  ARGUMENTS: NONE
//  Inserts a record into a table given a valid command with a table name and the values of the record
Table SQL::insert() {
    vector<string> table_name = p.parse_tree()["table_name"];
    vector<string> values = p.parse_tree()["values"];

    t = Table(table_name[0]);
    t.insert_into(values);
    return t;
}