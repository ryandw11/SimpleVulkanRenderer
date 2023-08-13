#pragma once
#ifndef QUEUE_H
#define QUEUE_H

#include "Node.h"
#include <iostream>

/**
 * This templated class represents the stack structure.
 * Class T is the data type that you want to store in the Stack.
 *
 * Note: T must override the == operator.
 */
template <class T>
class Queue {
private:
    // Keep track of the size and the start node (general node, not graph node).
    size_t size;
    genode::Node<T>* start;
    genode::Node<T>* end;

public:
    // The constructor and deconstructor.
    Queue();
    ~Queue();

    // Push, pop, and peek methods.
    void enqueue(T);
    T dequeue();
    T& peek();

    // Contains and length method.
    bool contains(T);
    bool isEmpty() const;
    size_t length() const;

    // Allow the direct use of std::cout << stack. (class T must also be a friend of << on the ostream.)
    template<class A>
    friend std::ostream& operator<<(std::ostream& out, const Queue<A>& self);

    /**
     * Get a pointer the start of the stack.
     *
     * @returns A pointer to the start of the stack.
     */
    genode::Node<T>* begin() const {
        return this->start;
    }
};

/**
 * This is the constructor for the Stack.
 */
template<class T>
Queue<T>::Queue() {
    this->size = 0;
    this->start = nullptr;
    this->end = nullptr;
}

/**
 * This is the deconstructor for the stack.
 */
template<class T>
Queue<T>::~Queue() {
    // Loop through all nodes and delete them.
    genode::Node<T>* next = this->start;
    while (next != nullptr) {
        genode::Node<T>* temp = next;
        next = next->getNextNode();
        delete temp;
    }
    this->start = nullptr;
    this->end = nullptr;
}

/**
 * Enqueue an item to the queue.
 *
 * @param item The item to enqueue. (Note: A copy will be made and the item cannot be modified anymore.)
 */
template<class T>
void Queue<T>::enqueue(T item) {
    if (this->start == nullptr) {
        this->start = new genode::Node<T>(item);
        this->end = this->start;
        this->size++;
        return;
    }

    genode::Node<T>* newNode = new genode::Node<T>(item);
    newNode->setNextNode(this->end);
    this->end = newNode;
    this->size++;
}

/**
 * Dequeue (remove) an item from the queue. The item is removed from the front of the queue.
 * Throws a runtime error if there is nothing left to dequeue.
 *
 * @returns The value that was stored at the front of the queue.
 */
template<class T>
T Queue<T>::dequeue() {
    if (this->start == nullptr) {
        throw std::runtime_error("There is nothing left to dequeue!");
    }

    genode::Node<T>* nodeToPop = this->start;

    T value = nodeToPop->getValue();

    this->start = nodeToPop->getNextNode();
    nodeToPop->setNextNode(nullptr);
    delete nodeToPop;
    this->size--;

    if (this->start == nullptr) {
        this->end = nullptr;
    }

    return value;
}

/**
 * Peek (look at) the item on top of the queue. The item WILL NOT be removed.
 * Throws a runtime error if there is nothing to peek.
 *
 * @returns A reference of the value at the front of the queue. (This can be modified).
 */
template<class T>
T& Queue<T>::peek() {
    if (this->start == nullptr) {
        throw std::runtime_error("There is nothing left to peek!");
    }

    return this->start->getRefValue();
}

/**
 * Check to see if an item is within the queue. (The value T is assumed to be a copy. T must also override ==.)
 *
 * @param value The value to search for. (A copy)
 *
 * @returns If the queue contains the desired item.
 */
template<class T>
bool Queue<T>::contains(T value) {
    genode::Node<T>* current = this->start;
    while (current != nullptr) {
        if (current->getValue() == value) {
            return true;
        }

        current = current->getNextNode();
    }

    return false;
}

/**
 * Get the length (size) of the queue.
 *
 * @returns The size of the queue.
 */
template<class T>
size_t Queue<T>::length() const {
    return this->size;
}

template<class T>
bool Queue<T>::isEmpty() const {
    return this->size == 0;
}

/**
 * Overload the << operator for ostreams.
 *
 * Note: This uses recursion in order to print out the stack in the desired order.
 *
 * This can be used for cout << queue << endl;
 */
template <class T>
std::ostream& operator<<(std::ostream& out, const Queue<T>& self) {
    if (self.size < 1) {
        out << "Empty";
        return out;
    }
    // Loop through all nodes and print them.
    genode::Node<T>* next = this->start;
    while (next != nullptr) {
        std::cout << next << ", ";
        next = next->getNextNode();
    }
    std::cout << std::endl;
}

#endif