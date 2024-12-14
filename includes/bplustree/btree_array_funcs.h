#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

using namespace std;

template <class T>
T maximal(const T& a, const T& b) {
    if (a > b)
        return a;
    return b;
}

template <class T>
void swap(T& a, T& b) {
    T temp = a;
    a = b;
    b = a;
}

template <class T>
int index_of_maximal(T data[], int n) {
    return (n - 1);
}

template <class T>
int first_ge(const T data[], int n, const T& entry) {
    for (int i = 0; i < n; i++) {
        if (data[i] >= entry) {
            return i;
        }
    }
    return n;
}

template <class T>
void ordered_insert(T data[], int& n, T entry) {

    int index = first_ge(data, n, entry);

    for (int i = n; i > index; i--) {
        data[i] = data[i - 1];
    }

    data[index] = entry;
    n++;

    return;
}

template <class T>
void attach_item(T data[], int& n, const T& entry) {
    data[n++] = entry;
}

template <class T>
void insert_item(T data[], int i, int& n, T entry) {
    int index = n++;
    for (; index > i; index--) {
        data[index] = data[index - 1];
    }
    data[index] = entry;
}

template <class T>
void detach_item(T data[], int& n, T& entry) {
    entry = data[--n];
}

template <class T>
void delete_item(T data[], int i, int& n, T& entry) {
    entry = data[i];

    while (i < n) {
        data[i] = data[i + 1];
        i++;
    }
    n--;
}

template <class T>
void merge(T data1[], int& n1, T data2[], int& n2) {
    int n = 0;
    int index = n1;
    while (index < n1 + n2) {
        data1[index++] = data2[n++];
    }
    n1 = index;
    n2 = 0;
}

template <class T>
void split(T data1[], int& n1, T data2[], int& n2) {
    n2 = n1 / 2;
    n1 = n1 - n2;
    for (int i = 0; i < n2; i++) {
        data2[i] = data1[i + n1];
    }
}

template <class T>
void copy_array(T dest[], const T src[], int& dest_size, int src_size) {
    for (int i = 0; i < src_size; i++) {
        dest[i] = src[i];
    }
    dest_size = src_size;
}

template <class T>
void print_array(const T data[], int n, int pos = -1) {
    for (int i = 0; i < n; i++) {
        cout << "[" << data[i] << "]";
    }
    cout << endl;
}

template <class T>
bool is_gt(const T data[], int n, const T& item) {

    for (int i = 0; i < n; i++) {
        if (item <= data[i]) {
            return false;
        }
    }
    return true;
}

template <class T>
bool is_le(const T data[], int n, const T& item) {

    for (int i = 0; i < n; i++) {
        if (item > data[i]) {
            return false;
        }
    }
    return true;
}

template <typename T>
ostream& operator <<(ostream& outs, const vector<T>& list) {
    for (T item : list) {
        outs << "[" << item << "]";
    }
    return outs;
}

template <typename T>
vector<T>& operator +=(vector<T>& list, const T& addme) {
    list.push_back(addme);
    return list;
}