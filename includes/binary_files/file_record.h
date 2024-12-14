#pragma once

#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <iomanip>

using namespace std;

struct FileRecord {
    FileRecord() {
        _record[0][0] = '\0';
        recno = -1;
    }

    FileRecord(vector<string> vec) {
        for (int i = 0; i < vec.size(); i++) {
            strncpy(_record[i], vec[i].c_str(), MAX);
            _record[i][MAX+1] = '\0';
        }

        recno = -1;
    }

    FileRecord(char str[]) {
        strncpy(_record[0], str, MAX);
        _record[0][MAX + 1] = '\0';
    }
    FileRecord(string s) {
        strncpy(_record[0], s.c_str(), MAX);
        _record[0][MAX + 1] = '\0';
    }

    vector<string> get_record();
    int _num_of_fields() const;

    long write(fstream& outs);
    long read(fstream& ins, long recno);  

    friend ostream& operator<<(ostream& outs, const FileRecord& r);


    static const int MAX = 100;
    int recno;
    char _record[MAX + 1][MAX + 1];
};


long FileRecord::write(fstream& outs) {
    outs.seekp(0, outs.end);

    long pos = outs.tellp();

    outs.write(&_record[0][0], sizeof(_record));

    return pos / sizeof(_record);
}

long FileRecord::read(fstream& ins, long recno) {
    long pos = recno * sizeof(_record);
    ins.seekg(pos, ios_base::beg);
    ins.read(&_record[0][0], sizeof(_record));

    _record[0][ins.gcount()] = '\0';
    return ins.gcount();
}

vector<string> FileRecord::get_record() {
    vector<string> vec;
    for (int i = 0; i < MAX; i++) {
        if (_record[i][0] != '\0') {
            vec.push_back(string(_record[i]));
        }
        else {
            break;
        }
    }

    return vec;
}

int FileRecord::_num_of_fields() const {
    int count = 0;
    for (int i = 0; i < MAX; i++) {
        if (_record[i][0] != '\0') {
            count++;
        }
        else {
            break;
        }
    }
    return count;
}

ostream& operator<<(ostream& outs, const FileRecord& r) {
    for (int i = 0; i < r._num_of_fields(); i++) {
        outs << setw(15) << r._record[i];
    }

    return outs;
}