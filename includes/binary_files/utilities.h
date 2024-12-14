#pragma once

bool file_exists(const char filename[]);
void open_fileRW(fstream& f, const char filename[]);
void open_fileW(fstream& f, const char filename[]);
void open_fileRW_text(fstream& f, const char filename[]);
void open_fileW_text(fstream& f, const char filename[]);

using namespace std;

bool file_exists(const char filename[]) {
    const bool debug = false;
    fstream ff;
    ff.open(filename,
        std::fstream::in | std::fstream::binary);
    if (ff.fail()) {
        if (debug) cout << "file_exists(): File does NOT exist: " << filename << endl;
        return false;
    }
    if (debug) cout << "file_exists(): File DOES exist: " << filename << endl;
    ff.close();
    return true;
}

void open_fileRW(fstream& f, const char filename[]){
    const bool debug = false;
    if (!file_exists(filename)) {
        f.open(filename, std::fstream::out | std::fstream::binary);
        if (f.fail()) {
            cout << "file open (RW) failed: with out|" << filename << "]" << endl;
            throw("file RW failed  ");
        }
        else {
            if (debug) cout << "open_fileRW: file created successfully: " << filename << endl;
        }
    }
    else {
        f.open(filename,
            std::fstream::in | std::fstream::out | std::fstream::binary);
        if (f.fail()) {
            cout << "file open (RW) failed. [" << filename << "]" << endl;
            throw("file failed to open.");
        }
    }

}
void open_fileW(fstream& f, const char filename[]) {
    f.open(filename,
        std::fstream::out | std::fstream::binary);
    if (f.fail()) {
        cout << "file open failed: " << filename << endl;
        throw("file failed to open.");
    }

}


void open_fileRW_text(fstream& f, const char filename[]){
    const bool debug = false;
    if (!file_exists(filename)) {
        f.open(filename, std::fstream::out);
        if (f.fail()) {
            cout << "file open (RW) failed: with out|" << filename << "]" << endl;
            throw("file RW failed  ");
        }
        else {
            if (debug) cout << "open_fileRW: file created successfully: " << filename << endl;
        }
    }
    else {
        f.open(filename, std::fstream::in | std::fstream::out);
        if (f.fail()) {
            cout << "file open (RW) failed. [" << filename << "]" << endl;
            throw("file failed to open.");
        }
    }
}

void open_fileW_text(fstream& f, const char filename[]) {
    f.open(filename, std::fstream::out);
    if (f.fail()) {
        cout << "file open failed: " << filename << endl;
        throw("file failed to open.");
    }

}



