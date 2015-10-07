/**
 * Doubly linked list.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#pragma once

#include "c11httpd/pre__.h"

namespace c11httpd {


// Doubly linked list.
template <typename T>
class link_t {
public:
	// "offset" indicates where the real object is located.
	explicit link_t(uintptr_t offset = 0) : m_offset(offset) {
		this->m_prev = this;
		this->m_next = this;
	}

	// Check if current node is in a list or not.
	bool linked() const {
		return this->m_prev != this;
	}

	// Remove current node from list but do NOT free memory.
	void unlink() {
		if (!this->linked()) {
			return;
		}

		this->m_prev->m_next = this->m_next;
		this->m_next->m_prev = this->m_prev;
		this->m_prev = this;
		this->m_next = this;
	}

	// Remove all nodes (except for "head node") from the list and free their memory.
	//
	// Note that the current node MUST be "head node" (the first node).
	void clear() {
		auto ptr = this->m_next;

		while (ptr != this) {
			auto old = ptr;
			ptr = ptr->m_next;

			delete old->get();
		}

		this->m_prev = this;
		this->m_next = this;
	}

	// Pop a node from the beginning of the list.
	//
	// Note that the current node MUST be "head node" (the first node).
	// If the list is empty, then null will be returned.
	link_t<T>* pop_front() {
		if (!this->linked()) {
			return 0;
		}

		auto node = this->m_next;
		this->m_next = node->m_next;
		node->m_next->m_prev = this;
		return node;
	}

	// Pop a node from the back of the list.
	//
	// Note that the current node MUST be "head node" (the first node).
	// If the list is empty, then null will be returned.
	link_t<T>* pop_back() {
		if (!this->linked()) {
			return 0;
		}

		auto node = this->m_prev;
		this->m_prev = node->m_prev;
		node->m_prev->m_next = this;
		return node;
	}

	// Insert a new node at the beginning of the list.
	//
	// Note that the current node MUST be "head node" (the first node).
	void push_front(link_t<T>* new_one) {
		new_one->m_prev = this;
		new_one->m_next = this->m_next;
		this->m_next->m_prev = new_one;
		this->m_next = new_one;
	}

	// Insert a new node at the end of the list.
	//
	// Note that the current node MUST be "head node" (the first node).
	void push_back(link_t<T>* new_one) {
		new_one->m_prev = this->m_prev;
		new_one->m_next = this;
		this->m_prev->m_next = new_one;
		this->m_prev = new_one;
	}

	// Get previous node.
	link_t<T>* prev() const {
		return this->m_prev;
	}

	// Get next node.
	link_t<T>* next() const {
		return this->m_next;
	}

	// Get the object that this node points to.
	//
	// The curent node MUST NOT be "head node" (the first node).
	T* get() const {
		return (T*)(uintptr_t(this) - this->m_offset);
	}

private:
	const uintptr_t m_offset;
	link_t<T>* m_prev;
	link_t<T>* m_next;
};

} // namespace c11httpd.


