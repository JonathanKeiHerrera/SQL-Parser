#pragma once

#include "bplustree.h"

#include <iostream>
using namespace std;

template <typename K, typename V>
struct Pair {
    K key;
    V value;

    Pair(const K& k = K(), const V& v = V()) : key(k), value(v) {}

    friend ostream& operator <<(ostream& outs, const Pair<K, V>& print_me) {
        outs << "(" << print_me.key << ", " << print_me.value << ")";
        return outs;
    }

    friend bool operator ==(const Pair<K, V>& lhs, const Pair<K, V>& rhs) {
        return lhs.key == rhs.key;
    }

    friend bool operator < (const Pair<K, V>& lhs, const Pair<K, V>& rhs) {
        return lhs.key < rhs.key;
    }

    friend bool operator > (const Pair<K, V>& lhs, const Pair<K, V>& rhs) {
        return lhs.key > rhs.key;
    }

    friend bool operator <= (const Pair<K, V>& lhs, const Pair<K, V>& rhs) {
        return lhs.key <= rhs.key;
    }

    friend bool operator >= (const Pair<K, V>& lhs, const Pair<K, V>& rhs) {
        return lhs.key >= rhs.key;
    }

    friend Pair<K, V> operator + (const Pair<K, V>& lhs, const Pair<K, V>& rhs) {
        return Pair<K, V>(lhs.key, lhs.value + rhs.value);
    }

};


template <typename K, typename V>
class Map {
public:
    typedef BPlusTree<Pair<K, V> > map_base;
    class Iterator {
    public:
        friend class Map;
        Iterator() {}

        Iterator(typename map_base::Iterator it) {
            _it = it;
        }

        Iterator operator ++(int unused) {
            _it++;
            return *this;
        }

        Iterator operator ++() {
            ++_it;
            return *this;
        }

        Pair<K, V> operator *() {
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

    Map();

    Iterator begin();
    Iterator end();

    int size() const;
    bool empty() const;

    V& operator[](const K& key);
    V& at(const K& key);

    void insert(const K& k, const V& v);
    void erase(const K& key);
    void clear();
    V get(const K& key);

    Iterator find(const K& key);
    bool contains(const Pair<K, V>& target) const;


    int count(const K& key);
    Iterator lower_bound(const K& key);
    Iterator upper_bound(const K& key);
    Iterator equal_range(const K& lower, const K& upper);

    bool is_valid() { return map.is_valid(); }

    friend ostream& operator<<(ostream& outs, const Map<K, V>& print_me) {
        outs << print_me.map << endl;
        return outs;
    }
private:
    int key_count;
    BPlusTree<Pair<K, V> > map;
};

template <class K, class V>
Map<K, V>::Map() {
    key_count = 0;
}


template <class K, class V>
typename Map<K, V>::Iterator Map<K, V>::begin() {
    typename map_base::Iterator it = map.begin();
    return Iterator(it);
}

template <class K, class V>
typename Map<K, V>::Iterator Map<K, V>::end() {
    return Iterator(NULL);
}



template <class K, class V>
int Map<K, V>::size() const {
    return map.size();
}

template <class K, class V>
bool Map<K, V>::empty() const {
    return map.empty();
}

template <class K, class V>
V& Map<K, V>::operator[](const K& key) {
    V v;
    Pair<K, V> P(key, v);
    return map.get(P).value;
}

template <class K, class V>
V& Map<K, V>::at(const K& key) {
    return map.get(key).value;
}

template <class K, class V>
void Map<K, V>::insert(const K& k, const V& v) {
    map.insert(Pair<K, V>(k, v));
    key_count++;
}

template <class K, class V>
void Map<K, V>::erase(const K& key) {
    map.remove(key);
}

template <class K, class V>
void Map<K, V>::clear() {
    map.clear_tree();
}

template <class K, class V>
V Map<K, V>::get(const K& key) {
    V v;
    Pair<K, V> P(key, v);
    return map.get(P).value;
}

template <class K, class V>//  Operations:
typename Map<K, V>::Iterator Map<K, V>::find(const K& key) {
    typename map_base::Iterator bpt_it = map.begin();

    for (; bpt_it != map.end(); bpt_it++) {
        if (*bpt_it == Pair<K, V>(key)) {
            break;
        }
    }

    typename Map<K, V>::Iterator map_it(bpt_it);
    return map_it;
}

template <class K, class V>
bool Map<K, V>::contains(const Pair<K, V>& target) const {
    return map.contains(target);
}

template <class K, class V>
int Map<K, V>::count(const K& key) {
    return map.size();
}

template <class K, class V>
typename Map<K, V>::Iterator Map<K, V>::lower_bound(const K& key) {
    typename map_base::Iterator bpt_it = map.begin();

    for (; bpt_it != map.end(); bpt_it++) {
        if (*bpt_it >= Pair<K, V>(key)) {
            break;
        }
    }

    typename Map<K, V>::Iterator map_it(bpt_it);
    return map_it;
}

template <class K, class V>
typename Map<K, V>::Iterator Map<K, V>::upper_bound(const K& key) {
    typename map_base::Iterator bpt_it = map.begin();

    for (; bpt_it != map.end(); bpt_it++) {
        if (*bpt_it > Pair<K, V>(key)) {
            break;
        }
    }

    typename Map<K, V>::Iterator map_it(bpt_it);
    return map_it;
}

template <class K, class V>
typename Map<K, V>::Iterator Map<K, V>::equal_range(const K& lower, const K& upper) {
    typename map_base::Iterator bpt_it = map.begin();

    for (; bpt_it != map.end(); bpt_it++) {
        if (*bpt_it >= lower && *bpt_it <= upper) {
            break;
        }
    }

    typename Map<K, V>::Iterator map_it(bpt_it);
    return map_it;
}