#pragma once

#include "bplustree.h"

#include <iostream>
#include <vector>

using namespace std;

template <typename K, typename V>
struct MPair {
    K key;
    vector<V> value_list;

    MPair(const K& k = K()) {
        key = k;
    }

    MPair(const K& k, const V& v) {
        key = k;
        value_list.push_back(v);
    }

    MPair(const K& k, const vector<V>& vlist) {
        key = k;
        value_list = vlist;
    }

    friend ostream& operator <<(ostream& outs, const MPair<K, V>& print_me) {
        for (int i = 0; i < print_me.value_list.size(); i++) {
            outs << "(" << print_me.key << ", " << print_me.value_list[i] << ") ";
        }
        return outs;
    }

    friend bool operator ==(const MPair<K, V>& lhs, const MPair<K, V>& rhs) {
        return lhs.key == rhs.key;
    }

    friend bool operator < (const MPair<K, V>& lhs, const MPair<K, V>& rhs) {
        return lhs.key < rhs.key;
    }

    friend bool operator <= (const MPair<K, V>& lhs, const MPair<K, V>& rhs) {
        return lhs.key <= rhs.key;
    }

    friend bool operator > (const MPair<K, V>& lhs, const MPair<K, V>& rhs) {
        return lhs.key > rhs.key;
    }

    friend bool operator >= (const MPair<K, V>& lhs, const MPair<K, V>& rhs) {
        return lhs.key >= rhs.key;
    }

    friend MPair<K, V> operator + (const MPair<K, V>& lhs, const MPair<K, V>& rhs) {
        return MPair<K, V>(lhs.key, lhs.value_list + rhs.value_list);
    }

    friend MPair<K, V>& operator +=(MPair<K, V>& lhs, const V& value) {
        lhs.value_list.push_back(value);
        return lhs;
    }
};

template <typename K, typename V>
class MMap
{
public:
    typedef BPlusTree<MPair<K, V> > map_base;
    class Iterator {
    public:
        friend class MMap;

        Iterator() {
        }

        Iterator(typename map_base::Iterator it) {
            _it = it;
        }

        Iterator operator ++(int unused) {
            _it++;
            return *this;
        }

        Iterator operator ++() {
            _it++;
            return *this;
        }

        MPair<K, V> operator *() {
            return *_it;
        }

        friend bool operator ==(const Iterator& lhs, const Iterator& rhs) {
            return lhs._it == rhs._it;
        }

        friend bool operator !=(const Iterator& lhs, const Iterator& rhs) {
            return !(lhs._it == rhs._it);
        }

    private:
        typename map_base::Iterator _it;
    };

    MMap();

    Iterator begin();
    Iterator end();

    int size() const;
    bool empty() const;

    const vector<V>& operator[](const K& key) const;
    vector<V>& operator[](const K& key);
    vector<V>& at(const K& key);

    void insert(const K& k, const V& v);
    void erase(const K& key);
    void clear();

    bool contains(const K& key) const;
    vector<V>& get(const K& key);

    Iterator find(const K& key);
    int count(const K& key);
    Iterator lower_bound(const K& key);
    Iterator upper_bound(const K& key);
    Iterator equal_range(const K& key);

    bool is_valid();

    void print_lookup();

    friend ostream& operator<<(ostream& outs, const MMap<K, V>& print_me) {
        outs << print_me.mmap << endl;
        return outs;
    }

private:
    BPlusTree<MPair<K, V> > mmap;
};


template <class K, class V>
MMap<K, V>::MMap() {}

template <class K, class V>
typename MMap<K, V>::Iterator MMap<K, V>::begin() {
    typename map_base::Iterator mmap_it = mmap.begin();
    return Iterator(mmap_it);
}

template <class K, class V>
typename MMap<K, V>::Iterator MMap<K, V>::end() {
    return Iterator(mmap.end());
}

template <class K, class V>
int MMap<K, V>::size() const {
    return mmap.size();
}

template <class K, class V>
bool MMap<K, V>::empty() const {
    return mmap.empty();
}

template <class K, class V>
const vector<V>& MMap<K, V>::operator[](const K& key) const {
    MPair<K, V> mpair(key);
    if (mmap.contains(mpair)) {
        return mmap.get(mpair).value_list;
    }
    vector<V> empty_vector;
    return empty_vector;
}

template <class K, class V>
vector<V>& MMap<K, V>::operator[](const K& key) {

    MPair<K, V> mpair(key);
    if (mmap.contains(mpair)) {
        return mmap.get(mpair).value_list;
    }
    MPair<K, V> new_mpair(key);
    mmap.insert(new_mpair);
    return mmap.get(new_mpair).value_list;

}

template <class K, class V>
vector<V>& MMap<K, V>::at(const K& key) {
    return mmap.get(key).value_list;
}

template <class K, class V>
void MMap<K, V>::insert(const K& k, const V& v) {
    mmap.insert(MPair<K, V>(k, v));
    return;
}

template <class K, class V>
void MMap<K, V>::erase(const K& key) {
    mmap.remove(key);
    return;
}

template <class K, class V>
void MMap<K, V>::clear() {
    mmap.clear_tree();
    return;
}

template <class K, class V>
bool MMap<K, V>::contains(const K& key) const {
    return mmap.contains(key);
}

template <class K, class V>
vector<V>& MMap<K, V>::get(const K& key) {
    MPair<K, V> mpair(key);
    if (mmap.contains(mpair)) {
        return mmap.get(mpair).value_list;
    }
}

template <class K, class V>
typename MMap<K, V>::Iterator MMap<K, V>::find(const K& key) {
    typename map_base::Iterator bpt_it = mmap.begin();

    for (; bpt_it != mmap.end(); bpt_it++) {
        if (*bpt_it == MPair<K, V>(key)) {
            break;
        }
    }

    typename MMap<K, V>::Iterator mmap_it(bpt_it);
    return mmap_it;
}

template <class K, class V>
int MMap<K, V>::count(const K& key) {
    MPair<K, V> mpair(key);
    if (mmap.contains(mpair)) {
        return mmap.get(mpair).value_list.size();
    }
    return 0;
}

template <class K, class V>
typename MMap<K, V>::Iterator MMap<K, V>::lower_bound(const K& key) {
    typename map_base::Iterator bpt_it = mmap.begin();

    for (; bpt_it != mmap.end(); bpt_it++) {
        if ((*bpt_it).key >= key) {
            break;
        }
    }

    typename MMap<K, V>::Iterator mmap_it(bpt_it);
    return mmap_it;
}

template <class K, class V>
typename MMap<K, V>::Iterator MMap<K, V>::upper_bound(const K& key) {
    typename map_base::Iterator bpt_it = mmap.begin();

    for (; bpt_it != mmap.end(); bpt_it++) {
        if (*bpt_it > key) {
            break;
        }
    }

    typename MMap<K, V>::Iterator mmap_it(bpt_it);
    return mmap_it;
}

template <class K, class V>
typename MMap<K, V>::Iterator MMap<K, V>::equal_range(const K& key) {}

template <class K, class V>
bool MMap<K, V>::is_valid() {
    return mmap.is_valid();
}

template <class K, class V>
void MMap<K, V>::print_lookup() {
    typename map_base::Iterator bpt_it = mmap.begin();
    for (; bpt_it != mmap.end(); bpt_it++) {
        cout << (*bpt_it).key << ": ";

        for (int i = 0; i < (*bpt_it).value_list.size(); i++) {
            cout << (*bpt_it).value_list[i] << "|";
        }
        cout << endl;
    }

}