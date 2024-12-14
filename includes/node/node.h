#pragma once

#include <iostream>

template <typename T>
struct node {
	T _item;
	node<T>* _next;
	node<T>* _prev;

	node(T item = T(), node<T>* next = NULL, node<T>* prev = NULL) {
		_item = item;
		_next = next;
		_prev = prev;
	}

	friend std::ostream& operator <<(std::ostream& outs, const node<T>& print_me) {
		outs << "[" << print_me._item << "]";
		return outs;
	}
};