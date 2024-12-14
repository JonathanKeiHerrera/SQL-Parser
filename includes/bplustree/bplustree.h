#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include "btree_array_funcs.h"

using namespace std;

template <class T>
class BPlusTree {
public:
    class Iterator {
    public:
        friend class BPlusTree;
        Iterator(BPlusTree<T>* _it = NULL, int _key_ptr = 0) : it(_it), key_ptr(_key_ptr) {}

        T operator *() {
            return it->data[key_ptr];
        }

        Iterator operator++(int un_used) {
            Iterator copy = *this;
            key_ptr++;
            if (key_ptr >= it->data_count) {
                it = it->next;
                key_ptr = 0;
            }
            return copy;
        }

        Iterator operator++() {
            key_ptr++;
            if (key_ptr >= it->data_count) {
                it = it->next;
                key_ptr = 0;
            }
            return *this;
        }

        friend bool operator ==(const Iterator& lhs, const Iterator& rhs) {
            // return (lhs.it->data[lhs.key_ptr] == rhs.it->data[rhs.key_ptr]);
            return (lhs.it == rhs.it && lhs.key_ptr == rhs.key_ptr);
        }

        friend bool operator !=(const Iterator& lhs, const Iterator& rhs) {
            // return (lhs.it != rhs.it);
            return (lhs.it != rhs.it);
        }

        void print_Iterator() {
            if (it) {
                cout << "iterator: ";
                print_array(it->data, it->data_count, key_ptr);
            }
            else {
                cout << "iterator: NULL, key_ptr: " << key_ptr << endl;
            }
        }
        bool is_null() { return !it; }
        void info() {
            cout << endl << "Iterator info:" << endl;
            cout << "key_ptr: " << key_ptr << endl;
            cout << "it: " << *it << endl;
        }

    private:
        BPlusTree<T>* it;
        int key_ptr;
    };

    BPlusTree(bool dups = false);
    BPlusTree(T* a, int size, bool dups = false);

    //big three:
    BPlusTree(const BPlusTree<T>& other);
    ~BPlusTree();
    BPlusTree<T>& operator =(const BPlusTree<T>& RHS);

    void copy_tree(const BPlusTree<T>& other);  //copy other into this object
    void copy_tree(const BPlusTree<T>& other, BPlusTree<T>*& last_node);

    void insert(const T& entry);   //insert entry into the tree
    void remove(const T& entry);   //remove entry from the tree
    void clear_tree();             //clear this object (delete all nodes etc.)

    bool contains(const T& entry) const; //true if entry can be found
    T& get(const T& entry);              //return a reference to entry
    // const T& get(const T& entry)const;   //return a reference to entry
    T& get_existing(const T& entry);     //return a reference to entry
    Iterator find(const T& key);         //return an iterator to this key.
    //     NULL if not there.
    Iterator lower_bound(const T& key);  //return first that goes NOT BEFORE
    // key entry or next if does not
    // exist: >= entry
    Iterator upper_bound(const T& key);  //return first that goes AFTER key
    //exist or not, the next entry  >entry

    int size() const;                    //count the number of elements

    bool empty() const;                  //true if the tree is empty

    void print_tree(int level = 0, ostream& outs = cout) const;

    friend ostream& operator<<(ostream& outs, const BPlusTree<T>& print_me) {
        print_me.print_tree(0, outs);
        return outs;
    }

    bool is_valid();
    string in_order();
    string pre_order();
    string post_order();

    Iterator begin();
    Iterator end();
    ostream& list_keys(Iterator from = NULL, Iterator to = NULL) {
        if (from == NULL) from = begin();
        if (to == NULL) to = end();
        for (Iterator it = from; it != to; it++)
            cout << *it << " ";
        return cout;

    }

private:
    static const int MINIMUM = 1;
    static const int MAXIMUM = 2 * MINIMUM;

    bool dups_ok;                        //true if duplicate keys are allowed
    int data_count;                      //number of data elements
    T data[MAXIMUM + 1];                 //holds the keys
    int child_count;                     //number of children
    BPlusTree* subset[MAXIMUM + 2];      //subtrees
    BPlusTree* next;
    bool is_leaf() const { return child_count == 0; }             //true if this is a leaf node

    T* find_ptr(const T& entry);         //return a pointer to this key.
    // NULL if not there.

//insert element functions
    void loose_insert(const T& entry);   //allows MAXIMUM+1 data elements in
    //   the root
    void fix_excess(int i);              //fix excess in child i

    //remove element functions:
    void loose_remove(const T& entry);  //allows MINIMUM-1 data elements
    //  in the root

    BPlusTree<T>* fix_shortage(int i);  //fix shortage in child i
    // and return the smallest key in this subtree
    BPlusTree<T>* get_smallest_node();
    void get_smallest(T& entry);      //entry := leftmost leaf
    void get_biggest(T& entry);       //entry := rightmost leaf
    void remove_biggest(T& entry);    //remove the biggest child of tree->entry
    void transfer_left(int i);        //transfer one element LEFT from child i
    void transfer_right(int i);       //transfer one element RIGHT from child i
    BPlusTree<T>* merge_with_next_subset(int i); //merge subset i with  i+1

};

template <typename T>
BPlusTree<T>::BPlusTree(bool dups) {
    data_count = 0;
    child_count = 0;
    next = nullptr;

    for (int i = 0; i < MAXIMUM + 2; i++) {
        subset[i] = nullptr;
    }
}

template <typename T>
BPlusTree<T>::BPlusTree(T* a, int size, bool dups) {
    data_count = 0;
    child_count = 0;
    next = nullptr;

    for (int i = 0; i < size; i++) {
        insert(a[i]);
    }
}

template <typename T>
BPlusTree<T>::BPlusTree(const BPlusTree<T>& other) {
    copy_tree(other);
}

template <typename T>
BPlusTree<T>::~BPlusTree() {
    clear_tree();
}

template <typename T>
BPlusTree<T>& BPlusTree<T>::operator =(const BPlusTree<T>& RHS) {
    copy_tree(RHS);

    return *this;
}


template <typename T>
void BPlusTree<T>::copy_tree(const BPlusTree<T>& other) {
    BPlusTree<T>* last_ptr = nullptr;
    copy_tree(other, last_ptr);

    return;
}

// TODO
template <typename T>
void BPlusTree<T>::copy_tree(const BPlusTree<T>& other, BPlusTree<T>*& last_node) {
    data_count = other.data_count;
    child_count = other.child_count;

    for (int i = 0; i < other.data_count; i++) {
        data[i] = other.data[i];
    }

    if (!other.is_leaf()) {
        for (int i = 0; i < other.child_count; i++) {
            subset[i] = new BPlusTree<T>;
            subset[i]->copy_tree(*other.subset[i], last_node);
        }
    }
    else {
        if (last_node != nullptr) {
            last_node->next = this;
        }
        last_node = this;
    }

}

template <typename T>
void BPlusTree<T>::insert(const T& entry) {
    // cout << "Inserting: " << entry << endl;
    loose_insert(entry);

    // IF there is excess at the root:
    if (data_count >= MAXIMUM + 1) {
        // Create a new BPlusTree node
        BPlusTree<T>* ptr = new BPlusTree<T>();

        // Shallow copy the subtrees
        ptr->data_count = data_count;
        for (int i = 0; i < data_count; i++) {
            ptr->data[i] = data[i];
        }

        ptr->child_count = child_count;
        for (int i = 0; i < child_count; i++) {
            ptr->subset[i] = subset[i];
        }

        data_count = 0;
        child_count = 1;
        subset[0] = ptr;

        fix_excess(0);
    }

    // cout << "================================\n";
    // cout << "After inserting: " << entry << endl;
    // print_tree();
    // cout << "================================\n";

    return;
}

template <typename T>
void BPlusTree<T>::remove(const T& entry) {
    loose_remove(entry);

    if (data_count < MINIMUM && child_count >= 1) {
        BPlusTree<T>* shrink_ptr = subset[0];

        data_count = shrink_ptr->data_count;
        for (int i = 0; i < data_count; i++) {
            data[i] = shrink_ptr->data[i];
        }

        child_count = shrink_ptr->child_count;
        for (int i = 0; i < child_count; i++) {
            subset[i] = shrink_ptr->subset[i];
        }

        shrink_ptr->child_count = 0;
        shrink_ptr->data_count = 0;
        delete shrink_ptr;
    }

    return;
}

template <typename T>
void BPlusTree<T>::clear_tree() {
    for (int i = 0; i < child_count; i++) {
        subset[i]->clear_tree();
        delete subset[i];
        subset[i] = nullptr;
    }

    child_count = 0;
    data_count = 0;

    return;
}

template <typename T>
bool BPlusTree<T>::contains(const T& entry) const {
    // FIND first index that is greater than or equal to entry:
    int index = first_ge(data, data_count, entry);

    if (data[index] == entry) {
        return true;
    }

    if (is_leaf()) {
        return false;
    }

    return subset[index]->contains(entry);
}

template <typename T>
T& BPlusTree<T>::get(const T& entry) {
    if (!contains(entry)) {
        insert(entry);
    }

    T& item = get_existing(entry);

    return item;
}

template <typename T>
T& BPlusTree<T>::get_existing(const T& entry) {
    int i = first_ge(data, data_count, entry);
    bool found = (i < data_count && data[i] == entry);

    if (is_leaf()) {
        if (found) {
            return data[i];
        }
        else {
            return data[i];
            // assert(found && "get existing was called with nonexistent entry");
        }
    }
    if (found) {
        return subset[i + 1]->get_existing(entry);
    }
    else {
        return subset[i]->get_existing(entry);
    }

    return data[i];
}

// TODO
template <typename T>
typename BPlusTree<T>::Iterator BPlusTree<T>::find(const T& entry) {
    // FIND first index that is greater than or equal to entry:
    int i = first_ge(data, data_count, entry);
    bool found = (i < data_count && data[i] == entry);

    if (found) {
        if (is_leaf()) {
            Iterator it(this, i);
            return it;
        }
        else {
            return subset[i + 1]->find(entry);
        }
    }
    else {
        if (is_leaf()) {
            return Iterator(0, 0);
        }
        return subset[i]->find(entry);
    }
}

// TODO
template <typename T>
typename BPlusTree<T>::Iterator BPlusTree<T>::lower_bound(const T& key) {
    typename BPlusTree<T>::Iterator it = begin();

    for (; it != end(); it++) {
        if (*it >= key) {
            break;
        }
    }

    return it;
}

// TODO
template <typename T>
typename BPlusTree<T>::Iterator BPlusTree<T>::upper_bound(const T& key) {
    typename BPlusTree<T>::Iterator it = begin();

    for (; it != end(); it++) {
        if (*it > key) {
            break;
        }
    }

    return it;
}

// TODO
template <typename T>
int BPlusTree<T>::size() const {
    int size = 0;
    if (is_leaf()) {
        size = data_count;
    }

    for (int i = 0; i < child_count; i++) {
        size += subset[i]->size();
    }

    return size;
}

template <typename T>
bool BPlusTree<T>::empty() const {
    return (data_count == 0);
}

template <typename T>
void BPlusTree<T>::print_tree(int level, ostream& outs) const {
    // Prototype 3
        // Print last subtree
    if (child_count > 0 && subset[child_count - 1] && child_count != 1) {
        subset[child_count - 1]->print_tree(level + 1, outs);
    }

    // print data items:
    outs << setw(4 * level) << "-" << endl;
    for (int i = data_count - 1; i >= 0; i--) {
        outs << setw(4 * level) << "" << data[i] << endl;
        if (i < child_count && i > 0) {
            subset[i]->print_tree(level + 1, outs);
        }
    }
    outs << setw(4 * level) << "-" << endl;

    // Print the smallest subtree
    if (child_count > 0 && subset[0]) {
        subset[0]->print_tree(level + 1, outs);
    }

    return;
}

template <typename T>
bool BPlusTree<T>::is_valid() {

    // Check that every data[i] < data[i+1]
    for (int i = 1; i < data_count; i++) {
        if (data[i - 1] >= data[i]) {
            return false;
        }
    }

    if (is_leaf()) {
        return true;
    }

    // data[data_count-1] must be less than equal to every subset[child_count-1]->data[]
    for (int i = 0; i < subset[child_count - 1]->data_count; i++) {
        if (data[data_count - 1] > subset[child_count - 1]->data[i]) {
            return false;
        }
    }

    // every data[i] is equal to subset[i+1]->smallest
    for (int i = 0; i < data_count; i++) {
        T smallest;
        subset[i + 1]->get_smallest(smallest);

        if (!(data[i] == smallest)) {
            return false;
        }
    }

    // Recursively validate every subset[i]
    bool success = true;
    for (int i = 0; i < child_count; i++) {
        success = subset[i]->is_valid();
    }

    return success;
}



template <typename T>
string BPlusTree<T>::in_order() {
    string str = "";

    if (child_count > 0 && subset[0]) {
        str += subset[0]->in_order();
    }

    for (int i = 0; i < data_count; i++) {
        str += to_string(data[i]);
        str += "|";
        if (i < child_count - 1) {
            str += subset[i + 1]->in_order();
        }
    }

    return str;
}

// TODO
template <typename T>
string BPlusTree<T>::pre_order() {
    string str = "";

    int j = 0;
    for (int i = 0; i < data_count; i++) {
        str += to_string(data[i]);
        str += "|";

        if (!is_leaf()) {
            str += subset[j]->pre_order();
            j++;
        }

        if (!is_leaf() && i == data_count - 1) {
            str += subset[j]->pre_order();
            j++;
        }
    }

    return str;
}

// TODO
template <typename T>
string BPlusTree<T>::post_order() {
    string str = "";

    for (int i = 0; i < data_count; i++) {
        if (!is_leaf() && (i == 0 || i == data_count - 1)) {
            str += subset[i]->post_order();
        }
    }

    for (int i = 0; i < data_count; i++) {
        if (!is_leaf() && i == data_count - 1) {
            str += subset[i + 1]->post_order();
        }
        str += to_string(data[i]) + "|";
    }

    return str;
}

// TODO
template <typename T>
typename BPlusTree<T>::Iterator BPlusTree<T>::begin() {
    if (data_count == 0) {
        return Iterator(NULL);
    }
    Iterator it(get_smallest_node(), 0);
    return it;
}

// TODO
template <typename T>
typename BPlusTree<T>::Iterator BPlusTree<T>::end() {
    return Iterator(NULL);
}

// TODO
template <typename T>
T* BPlusTree<T>::find_ptr(const T& entry) {
    int i = first_ge(data, data_count, entry);
    bool found = (i < data_count && data[i] == entry);

    if (found) {
        return &data[i];
    }
    else {
        return nullptr;
    }
}


template <typename T>
void BPlusTree<T>::loose_insert(const T& entry) {
    int i = first_ge(data, data_count, entry);
    bool found = (i < data_count && data[i] == entry);

    if (found && is_leaf()) { // It's a duplicate
        data[i] = entry;
        return;
    }

    if (found && !is_leaf()) {
        subset[i + 1]->loose_insert(entry);
        fix_excess(i);
    }

    if (!found && is_leaf()) {
        insert_item(data, i, data_count, entry);
    }

    if (!found && !is_leaf()) {
        subset[i]->loose_insert(entry);
        fix_excess(i);
    }

    return;
}

template <typename T>
void BPlusTree<T>::fix_excess(int i) {
    if (subset[i]->data_count <= MAXIMUM) {
        return;
    }

    insert_item(subset, i + 1, child_count, new BPlusTree<T>());
    split(subset[i]->data, subset[i]->data_count, subset[i + 1]->data, subset[i + 1]->data_count);
    split(subset[i]->subset, subset[i]->child_count, subset[i + 1]->subset, subset[i + 1]->child_count);

    T item;
    detach_item(subset[i]->data, subset[i]->data_count, item);
    ordered_insert(data, data_count, item);
    if (subset[i]->is_leaf()) {
        ordered_insert(subset[i + 1]->data, subset[i + 1]->data_count, item);
    }


    if (subset[i]->is_leaf()) {
        subset[i + 1]->next = subset[i]->next;
        subset[i]->next = subset[i + 1];
    }
    return;
}

template <typename T>
void BPlusTree<T>::loose_remove(const T& entry) {
    int i = first_ge(data, data_count, entry);
    bool found = (i < data_count && data[i] == entry);

    T found_entry;
    if (is_leaf()) {
        if (!found) {
        }
        else {
            // cout << "is_leaf, found" << endl;
            // cout << "deleting entry: " << entry << endl;
            delete_item(data, i, data_count, found_entry);
        }
    }
    else {
        if (!found) {
            // cout << "not leaf, not found" << endl;
            subset[i]->loose_remove(entry);
            fix_shortage(i);
        }
        else {
            subset[i + 1]->loose_remove(entry);

            fix_shortage(i + 1);

            if (data_count < MINIMUM) {

                int search_index;
                if (data_count == 0) {
                    search_index = first_ge(subset[0]->data, subset[0]->data_count, entry);

                    if (subset[0]->data[search_index] == entry && search_index < subset[0]->data_count) {
                        T smallest;
                        subset[0]->subset[search_index + 1]->get_smallest(smallest);

                        subset[0]->data[search_index] = smallest;
                    }
                    else {
                    }

                }
                else {
                    search_index = first_ge(data, data_count, entry);

                    // Search data for entry
                        // FOUND -> replace entry with smallest
                        // !FOUND -> search subset[i]->data
                    int search_index = first_ge(data, data_count, entry);

                    if (data[search_index] == entry && search_index < data_count) {
                        // If the entry is in data && we are not going out of bounds
                        data[search_index] = subset[search_index + 1]->get_smallest_node()->data[0];
                    }
                    else {
                        // maybe the entry is in a subset...
                        int check_value = first_ge(subset[search_index]->data, subset[search_index]->data_count, entry);

                        // If we find it, we will replace the thingy
                        if (subset[search_index]->data[check_value] == entry && check_value < subset[search_index]->data_count) {
                            T smallest;
                            subset[search_index]->subset[check_value + 1]->get_smallest(smallest);
                            subset[search_index]->data[check_value] = smallest;
                        }
                        else {
                            // first make sure that search_index isnt zero...
                            if (search_index > 0) {
                                // search the left subset instead
                                check_value = first_ge(subset[search_index - 1]->data, subset[search_index - 1]->data_count, entry);
                                // did we find?
                                if (subset[search_index - 1]->data[check_value] == entry && check_value < subset[search_index - 1]->data_count) {
                                    T smallest;
                                    subset[search_index - 1]->subset[check_value]->get_smallest(smallest);
                                    subset[search_index - 1]->data[check_value] = smallest;
                                }
                            }
                        }
                    }

                }

            }
            else {
                // Search data for entry
                    // FOUND -> replace entry with smallest
                    // !FOUND -> search subset[i]->data
                int search_index = first_ge(data, data_count, entry);

                if (data[search_index] == entry && search_index < data_count) {
                    // If the entry is in data && we are not going out of bounds
                    data[search_index] = subset[search_index + 1]->get_smallest_node()->data[0];
                }
                else {
                    // maybe the entry is in a subset...
                    int check_value = first_ge(subset[search_index]->data, subset[search_index]->data_count, entry);

                    // If we find it, we will replace the thingy
                    if (subset[search_index]->data[check_value] == entry && check_value < subset[search_index]->data_count) {
                        T smallest = subset[search_index]->subset[check_value + 1]->get_smallest_node()->data[0];
                        subset[search_index]->data[check_value] = smallest;
                    }
                    else {
                        // first make sure that search_index isnt zero...
                        if (search_index > 0) {
                            // search the left subset instead
                            check_value = first_ge(subset[search_index - 1]->data, subset[search_index - 1]->data_count, entry);
                            // did we find?
                            if (subset[search_index - 1]->data[check_value] == entry && check_value < subset[search_index - 1]->data_count) {
                                T smallest;
                                subset[search_index - 1]->subset[check_value]->get_smallest(smallest);
                                subset[search_index - 1]->data[check_value] = smallest;
                            }
                        }

                    }
                }
            }

        }
    }

    return;
}

template <typename T>
BPlusTree<T>* BPlusTree<T>::fix_shortage(int i) {
    if (subset[i]->data_count >= MINIMUM) {
        return subset[i];
    }

    if (i > 0 && subset[i - 1]->data_count > MINIMUM) {
        transfer_right(i - 1);
    }
    else if (i < child_count - 1 && subset[i + 1]->data_count > MINIMUM) {
        transfer_left(i + 1);
    }
    else if (i > 0) {
        merge_with_next_subset(i - 1);
    }
    else {
        merge_with_next_subset(i);
    }
    return subset[i];

}

template <typename T>
BPlusTree<T>* BPlusTree<T>::get_smallest_node() {
    if (is_leaf()) {
        return this;
    }
    return subset[0]->get_smallest_node();
}

template <typename T>
void BPlusTree<T>::get_smallest(T& entry) {
    if (is_leaf()) {
        entry = data[0];
    }
    else {
        subset[0]->get_smallest(entry);
    }
}

template <typename T>
void BPlusTree<T>::get_biggest(T& entry) {
    if (is_leaf()) {
        entry = data[data_count - 1];
    }
    else {
        subset[child_count - 1]->get_biggest(entry);
    }

}

template <typename T>
void BPlusTree<T>::remove_biggest(T& entry) {

}

template <typename T>
void BPlusTree<T>::transfer_left(int i) {
    if (!subset[i]->is_leaf()) {
        T item;

        delete_item(data, i - 1, data_count, item);
        attach_item(subset[i - 1]->data, subset[i - 1]->data_count, item);

        delete_item(subset[i]->data, 0, subset[i]->data_count, item);
        ordered_insert(data, data_count, item);

        if (!subset[i]->is_leaf()) {
            BPlusTree<T>* ptr;
            delete_item(subset[i]->subset, 0, subset[i]->child_count, ptr);
            attach_item(subset[i - 1]->subset, subset[i - 1]->child_count, ptr);
        }
    }
    else {
        T item;
        delete_item(subset[i]->data, 0, subset[i]->data_count, item);
        attach_item(subset[i - 1]->data, subset[i - 1]->data_count, item);

        data[i - 1] = subset[i]->data[0];

        // Updating next pointer: prob not needed
        subset[i - 1]->next = subset[i];
    }
}

template <typename T>
void BPlusTree<T>::transfer_right(int i) {

    if (!subset[i]->is_leaf()) {
        T item;
        detach_item(subset[i]->data, subset[i]->data_count, item);
        ordered_insert(data, data_count, item);

        delete_item(data, i + 1, data_count, item);
        insert_item(subset[i + 1]->data, 0, subset[i + 1]->data_count, item);
        if (!subset[i]->is_leaf()) {
            BPlusTree<T>* ptr;
            detach_item(subset[i]->subset, subset[i]->child_count, ptr);
            insert_item(subset[i + 1]->subset, 0, subset[i + 1]->child_count, ptr);
        }
    }
    else {
        T item;
        detach_item(subset[i]->data, subset[i]->data_count, item);
        insert_item(subset[i + 1]->data, 0, subset[i + 1]->data_count, item);
        data[i] = subset[i + 1]->data[0];

        subset[i]->next = subset[i + 1];
    }

}

template <typename T>
BPlusTree<T>* BPlusTree<T>::merge_with_next_subset(int i) {
    // Remove data[i] from this object
    T item;
    delete_item(data, i, data_count, item);

    // Append it to child[i]->data (if not leaf)
    if (!subset[i]->is_leaf()) {
        attach_item(subset[i]->data, subset[i]->data_count, item);
    }

    // Move all data items from susbet[i+1]->data to subset[i]->data
    merge(subset[i]->data, subset[i]->data_count, subset[i + 1]->data, subset[i + 1]->data_count);

    // Move all subset pointers from subset[i+1]->subset to subset[i]->subset
    merge(subset[i]->subset, subset[i]->child_count, subset[i + 1]->subset, subset[i + 1]->child_count);

    // Delete the extra subtree
    BPlusTree<T>* ptr;
    delete_item(subset, i + 1, child_count, ptr);

    if (subset[i]->is_leaf()) {
        subset[i]->next = ptr->next;
    }

    delete ptr;

    return subset[i];
}
