#pragma once

#include "../node/node.h"

template <typename T>
class Stack {
public:
	class Iterator {
		friend class Stack;

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

		friend Iterator operator++(Iterator& it, int unused) {
			++it;
			return it;
		}
	private:
		node<T>* _ptr;
	};

	Stack() {
		_top = nullptr;
		_size = 0;
	}

	Stack(const Stack<T>& copy_me) {
		_top = nullptr;
		_size = 0;

		node<T>* _current_node = copy_me._top;
		node<T>* _previous_node = nullptr;

		while (_current_node != nullptr) {
			node<T>* _new_node = new node<T>;
			_new_node->_item = _current_node->_item;
			_new_node->_next = nullptr;

			if (_top == nullptr) {
				_top = _new_node;
			}
			else {
				_previous_node->_next = _new_node;
			}
			_previous_node = _new_node;
			_current_node = _current_node->_next;
		}
		_size = copy_me._size;
	}

	~Stack() {
		while (!empty()) {
			pop();
		}
	}

	Stack<T>& operator =(const Stack<T>& RHS) {
		if (this == &RHS) {
			return *this;
		}

		while (!empty()) {
			pop();
		}

		node<T>* _current_node = RHS._top;
		node<T>* _previous_node = nullptr;
		while (_current_node != nullptr) {
			node<T>* _new_node = new node<T>;
			_new_node->_item = _current_node->_item;
			_new_node->_next = nullptr;

			if (_top == nullptr) {
				_top = _new_node;
			}
			else {
				_previous_node->_next = _new_node;
			}
			_previous_node = _new_node;
			_current_node = _current_node->_next;
		}
		_size = RHS._size;
		return *this;
	}

	T top() {
		if (empty()) {
			throw std::out_of_range("top(): stack is empty");
		}
		return _top->_item;
	}

	bool empty() {
		return _top == nullptr;
	}

	void push(T item) {
		node<T>* _new_node = nullptr;
		try {
			_new_node = new node<T>;
			_new_node->_item = item;
			_new_node->_next = _top;
			_top = _new_node;
			_size++;
		}
		catch (...) {
			delete _new_node;
			throw;
		}
	}

	T pop() {
		if (empty()) {
			throw std::runtime_error("pop(): stack is empty");
		}
		node<T>* _temp_node = _top;
		T item = _top->_item;
		_top = _top->_next;
		delete _temp_node;
		_size--;
		return item;
	}

	friend std::ostream& operator <<(std::ostream& outs, const Stack<T>& print_me) {
		node<T>* iter = print_me._top;
		while (iter != nullptr) {
			outs << "[" << iter->_item << "]";
			iter = iter->_next;
		}
		return outs;
	}

	Iterator begin() const {
		return Iterator(_top);
	}

	Iterator end() const {
		return nullptr;
	}

	int size() const {
		return _size;
	}

private:
	node<T>* _top;
	int _size;
};