#ifndef TABLE_H
#define TABLE_H

#include "includes/binary_files/file_record.h"
#include "includes/binary_files/utilities.h"
#include "includes/bplustree/bplustree.h"
#include "includes/bplustree/map.h"
#include "includes/bplustree/multimap.h"
#include "includes/queue/queue.h"
#include "includes/stack/stack.h"
#include "includes/tokenizer/token.h"
#include "includes/shunting_yard/shuntingyard.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <cassert>

using namespace std;


class Table {
public:
    Table();
    Table(const string& name);
    Table(const string& name, const vector<string>& fields);

    void set_fields(const vector<string>& fields);
    vector<string> get_fields();
    bool is_empty();
    int insert_into(const vector<string>& fields);

    Table select_all();
    Table select_all(vector<string> string);
    Table select(const vector<string>& fields, Queue<Token*>& postfix);
    Table select(const vector<string>& fields, const vector<string>& condition);
    Table select(const vector<string>& fields, const string& field, const string& op, const string& value);

    int get_token_type(const string& s);
    void reindex();
    friend ostream& operator << (ostream& outs, const Table& table);
    vector<long> cond(Queue<Token*>& post);
    Table vector_to_table(const vector<string>& fields, const vector<long>& vector_of_recnos);

    vector<long> select_recnos() { return _select_recnos; }
    vector<string> get_order(vector<string> fields, vector<string> record);
    void adjust_size(vector<string> fields, vector<string>& record);

    vector<MMap<string, long>>& get_indices() { return _indices; }
    Map<string, long>& get_field_map() { return _field_map; }
    int get_num_fields() { return _field_map.size(); }
    long get_last_rec() { return _last_record; }

    string get_file() { return _file_name; }
    string get_field_file() { return _field_file_name; }

private:
    vector<long> _select_recnos;
    static int serial;

    string _name;
    string _file_name;
    string _field_file_name;
    Map<string, long> _keywords;
    vector<MMap<string, long>> _indices;
    bool _empty;
    vector<string> _field_names;

    Map<string, long> _field_map;
    long _last_record;

};

int Table::serial = 0;

Table::Table() {
    _empty = true;
    _last_record = 0;
    _name = "table";
    _file_name = _name + ".bin";
    _field_file_name = _name + "_fields.txt";
    //serial++;

    fstream f;
    open_fileW(f, _file_name.c_str());
    f.close();
}

Table::Table(const string& name) {
    _empty = true;
    _last_record = 0;
    _name = name;
    _file_name = _name + ".bin";
    _field_file_name = _name + "_fields.txt";
    //serial++;

    // Retrieve fields from field file: insert into field_names
    fstream f;
    open_fileRW_text(f, _field_file_name.c_str());
    vector<string> fields;
    string hold = "";
    while (f >> hold) {
        fields.push_back(hold);
    }

    f.close();
    set_fields(fields);

    reindex();

    // Updates indices and last rec number
    FileRecord record;
    open_fileRW(f, _file_name.c_str());
    long recno = 0;
    while (record.read(f, recno) != 0) {
        _last_record++;
        recno++;
    }

    f.close();
}

Table::Table(const string& name, const vector<string>& fields) {
    //serial++;
    _empty = false;
    _last_record = 0;

    _name = name;
    _file_name = _name + ".bin";
    _field_file_name = _name + "_fields.txt";

    set_fields(fields);

    _indices.resize(fields.size());
    for (int i = 0; i < fields.size(); i++) {
        _indices[i] = MMap<string, long>();
    }

    fstream f;
    open_fileW(f, _file_name.c_str());
    f.close();
}

void Table::set_fields(const vector<string>& fields) {
    _field_names = fields;

    _field_map.clear();

    for (int i = 0; i < fields.size(); i++) {
        _field_map[fields[i]] = i;
    }

    fstream f;
    open_fileW_text(f, _field_file_name.c_str());
    for (string x : fields) {
        f << x << "\n";
    }
    f.close();
}

vector<string> Table::get_fields() {
    return _field_names;
}

bool Table::is_empty() {
    if (_select_recnos.size() == 0) return true;
    return false;
}

int Table::insert_into(const vector<string>& fields) {
    // 1. Write into binary file: 
    fstream f;
    open_fileRW(f, _file_name.c_str());

    FileRecord record = FileRecord(fields);
    _last_record = record.write(f);

    f.close();

    // 2. Return record # up by one so it doesnt screw up other stuff i think?
    long recno = _last_record++;

    // 3. Update indices   

    for (int i = 0; i < fields.size(); i++) {
        string key = fields[i];
        _indices[i][key].push_back(recno);
    }

    return recno;
}

Table Table::select_all() {
    Table result(_name + to_string(serial), _field_names);
    serial++;
    fstream f;
    open_fileRW(f, _file_name.c_str());

    // insert all records into the file or something?
    FileRecord record;
    long recno = 0;
    while (record.read(f, recno) != 0) {
        result.insert_into(record.get_record());
        _select_recnos.push_back(recno);
        recno++;
    }
    result._select_recnos = _select_recnos;

    f.close();
    return result;
}

Table Table::select_all(vector<string> fields) {

    Table result(_name + to_string(serial), fields);
    serial++;
    fstream f;
    open_fileRW(f, _file_name.c_str());

    FileRecord record;
    long recno = 0;
    while (record.read(f, recno) != 0) {
        vector<string> insert_record = get_order(fields, record.get_record());

        _select_recnos.push_back(recno);
        result.insert_into(insert_record);
        recno++;
    }
    result._select_recnos = _select_recnos;
    f.close();

    return result;
}


Table Table::select(const vector<string>& fields, Queue<Token*>& postfix) {
    _select_recnos = cond(postfix);
    return vector_to_table(fields, _select_recnos);
}

Table Table::select(const vector<string>& fields, const vector<string>& condition) {
    Queue<Token*> infix;
    for (string x : condition) {
        Token* t = nullptr;
        switch (get_token_type(x)) {
        case LEFT_PAR:
            t = new LeftPar();
            break;
        case RIGHT_PAR:
            t = new RightPar();
            break;
        case OR:
        case AND:
        case LOGICAL:
            t = new Logical(x);
            break;
        case RELATIONAL:
            t = new Relational(x);
            break;
        case TOKEN_STR:
            t = new TokenStr(x);
            break;
        case RESULT_SET:
            t = new ResultSet(x);
            break;
        case OPERATOR:
            t = new Operator(x);
        default:
            break;
        }
        infix.push(t);
    }

    ShuntingYard sy(infix);
    Queue<Token*> postfix = sy.postfix();

    // Give postfix queue to other select functions
    return select(fields, postfix);
}

Table Table::select(const vector<string>& fields, const string& field,
    const string& op, const string& value) {

    _select_recnos.clear();
    // Gives us index for the field type:
    int index = _field_map.get(field);

    MMap<string, long>::Iterator it = _indices[index].begin();
    MMap<string, long>::Iterator end = _indices[index].end();

    // Find out which operator we do:
    if (op == "=") {
        _select_recnos = _indices[index][value];
        Table result = vector_to_table(fields, select_recnos());
        return result;
    }
    else if (op == ">=") {
        it = _indices[index].MMap<string, long>::lower_bound(value);

    }
    else if (op == ">") {
        it = _indices[index].MMap<string, long>::upper_bound(value);

    }
    else if (op == "<=") {
        end = _indices[index].MMap<string, long>::upper_bound(value);

    }
    else if (op == "<") {
        end = _indices[index].MMap<string, long>::lower_bound(value);
    }

    for (; it != end; it++) {
        vector<long> next = (*it).value_list;
        for (const auto x : next) {
            _select_recnos.push_back(x);
        }
    }

    Table result = vector_to_table(fields, select_recnos());
    return result;
}

int Table::get_token_type(const string& s) {
    if (s == "=" || s == "<" || s == ">" || s == "<=" || s == ">=") {
        return RELATIONAL;
    }
    else if (s == "and") {
        return AND;
    }
    else if (s == "or") {
        return OR;
    }
    else if (s == "(") {
        return LEFT_PAR;
    }
    else if (s == ")") {
        return RIGHT_PAR;
    }
    else {
        return TOKEN_STR;
    }
}

void Table::reindex() {
    // Clear indices
    _indices.clear();
    _indices.resize(_field_names.size());

    fstream f;
    open_fileRW(f, _file_name.c_str());

    FileRecord record;
    long recno = 0;
    while (record.read(f, recno) != 0) {
        for (int i = 0; i < _field_names.size(); i++) {
            string key = record.get_record()[i];
            _indices[i][key].push_back(recno);
        }
        recno++;
    }
    f.close();

}

ostream& operator << (ostream& outs, const Table& table) {
    outs << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
    outs << "Table name: " << table._file_name << ", records: " << table._last_record << endl;
    outs << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";

    outs << "Records";
    for (int i = 0; i < table._field_names.size(); i++) {
        outs << setw(15) << table._field_names[i];
    }
    outs << endl << "....................................................................................................\n";

    // open some file for reading records
    fstream f;
    open_fileRW(f, table._file_name.c_str());

    FileRecord record;
    long recno = 0;
    while (record.read(f, recno) != 0) {
        outs << setw(7) << recno;
        outs << record << endl;
        recno++;
    }
    outs << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n";

    f.close();
    return outs;
}

vector<long> Table::cond(Queue<Token*>& post) {

    // Given a postfix queue, return a vector<long> of record #s
    Stack<Token*> stack;
    ResultSet* res = nullptr;
    Token* left = nullptr;
    Token* right = nullptr;

    while (!post.empty()) {
        Token* t_ptr = post.pop();

        switch (t_ptr->type_of()) {
        case TOKEN_STR:
            stack.push(t_ptr);
            break;

        case RELATIONAL:
            assert(stack.size() >= 2 && "stack less than 2");
            right = stack.pop();
            left = stack.pop();

            if (_field_map.contains(left->token_str())) {
                res = static_cast<Relational*>(t_ptr)->eval(_indices[_field_map.get(left->token_str())], left, right);
            }
            else {
                vector<long> empty;
                res->set_records(empty);
            }

            stack.push(new ResultSet(*res));
            break;

        case LOGICAL:
        case AND:
        case OR:
            assert(stack.size() >= 2 && "Stack less than 2");
            right = stack.pop();
            left = stack.pop();

            res = static_cast<Logical*>(t_ptr)->eval(left, right);

            stack.push(new ResultSet(*res));
            break;
        default:
            cout << "WHAT DID YOU FIND????" << endl;
            break;
        }
    }

    // Now check the stack:
    //  Pop stack one more time to get the final resultset
    //  Retrieve resultset's vector<long>
    //  Return it
    // Dont forget to assert empty() or something

    assert(!stack.empty() && "STACK IS EMPTY");
    Token* t_ptr = stack.pop();
    assert(stack.empty() && "STACK IS NOT EMPTY");

    vector<long> record_numbers = static_cast<ResultSet*>(t_ptr)->get_records();

    delete static_cast<ResultSet*>(t_ptr);

    return record_numbers;
}

Table Table::vector_to_table(const vector<string>& fields, const vector<long>& vector_of_recnos) {
    Table t(_name + to_string(serial), fields);
    serial++;

    FileRecord record;
    fstream f;
    open_fileRW(f, _file_name.c_str());

    int bound = vector_of_recnos.size();
    for (int i = 0; i < bound; i++) {
        record.read(f, vector_of_recnos[i]);

        vector<string> insert_record = get_order(fields, record.get_record());

        if (insert_record.size() != fields.size()) {
            adjust_size(fields, insert_record);
        }

        t.insert_into(insert_record);
    }

    t._select_recnos = _select_recnos;


    f.close();
    return t;
}

vector<string> Table::get_order(vector<string> fields, vector<string> record) {
    vector<string> result;
    // Get the proper order from fields and the fields map, put it into a vector of ints
    for (string x : fields) {
        if (_field_map.contains(x)) {
            result.push_back(record[_field_map.get(x)]);
        }
        else {
            result.push_back(record[0]);
        }
    }
    return result;
}

void Table::adjust_size(vector<string> fields, vector<string>& record) {
    vector<long> wanted_fields;
    for (string x : fields) {
        wanted_fields.push_back(_field_map.get(x));
    }
    vector<string> temp = record;
    record.clear();
    for (int i = 0; i < wanted_fields.size(); i++) {
        record[i] = temp[wanted_fields[i]];
    }
}

#endif