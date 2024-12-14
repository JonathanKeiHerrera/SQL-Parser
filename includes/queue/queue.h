#pragma once

#include "../node/node.h"

template <typename T>
class Queue {
public:
	class Iterator {
	public:
		Iterator() {
			_ptr = nullptr;
		}

		Iterator(node<T>* p) {
			_ptr = p;
		}

		T operator *() const {
			if (_ptr == nullptr) {
				throw std::out_of_range("Iterator out of range");
			}
			return (_ptr->_item);
		}

		bool is_null() const {
			return (_ptr == nullptr);
		}

		friend bool operator !=(const Iterator& left, const Iterator& right) {
			return (left._ptr != right._ptr);
		}

		friend bool operator ==(const Iterator& left, const Iterator& right) {
			return (left._ptr == right._ptr);
		}

		Iterator& operator++() {
			_ptr = _ptr->_next;
			return *this;
		}

		friend Iterator operator++(Iterator& it,
			int unused) {
			++it;
			return it;
		}

	private:
		node<T>* _ptr;
	};

	Queue() {
		_front = nullptr;
		_rear = nullptr;
		_size = 0;
	}

	Queue(const Queue<T>& copy_me) {
		_front = nullptr;
		_rear = nullptr;
		_size = 0;

		node<T>* iter = copy_me._front;
		while (iter != nullptr) {
			push(iter->_item);
			iter = iter->_next;
		}
	}

	~Queue() {
		while (!empty()) {
			pop();
		}
	}

	Queue& operator =(const Queue<T>& RHS) {
		// Case 1: already equal
		if (this == &RHS) {
			return *this;
		}

		// Case 2: Not the same
		// a) clear the queue
		while (!empty()) {
			pop();
		}

		// b) copy queue elements
		node<T>* temp = RHS._front;
		while (temp != nullptr) {
			push(temp->_item);
			temp = temp->_next;
		}

		// c) copy queue size
		_size = RHS._size;

		return *this;
	}

	bool empty() const {
		return _front == nullptr;
	}

	T front() const {
		if (empty()) {
			throw std::out_of_range("front(): queue is empty");
		}
		return _rear->_item;
	}

	void push(T item) {
		node<T>* _new_node = new node<T>;
		_new_node->_item = item;
		if (empty()) {
			_front = _new_node;
			_rear = _new_node;
		}
		else {
			_rear->_next = _new_node;
			_rear = _new_node;
		}
		_size++;
	}

	T pop() {
		if (empty()) {
			throw std::out_of_range("pop(): queue is empty");
		}

		node<T>* temp = _front;
		T item = _front->_item;
		_front = _front->_next;

		if (_front == nullptr) {
			_rear = nullptr;
		}
		delete temp;
		_size--;
		return item;
	}

	Queue<T>::Iterator begin() const {
		return Iterator(_front);
	}

	Iterator end() const {
		return nullptr;
	}

	void print_pointers() const {
		node<T>* iter = _front;
		while (iter != nullptr) {
			std::cout << iter << " -> ";
			iter = iter->_next;
		}
		std::cout << "nullptr" << std::endl;
	}

	int size() const {
		return _size;
	}

	/*template <typename TT>
	friend std::ostream& operator <<(std::ostream& outs, const Queue<TT>& print_me) {
		node<TT>* iter = print_me._front;
		while (iter != nullptr) {
			outs << *iter->_item << " ";
			iter = iter->_next;
		}
		return outs;
	}*/

private:
	node<T>* _front;
	node<T>* _rear;
	int _size;
};