/*
    Author: Ryandw11
    Description: This is the header file that contains the general node used by the queue data structure.
*/

#pragma once
#ifndef NODE_H
#define NODE_H
// This namespace denotes the difference between a graph node and a datastructure (used by linkedlist and stack) node.
// I also cannot think of a better name than Node for the LinkedList and stack, thus I use this instead.
// genode stands for general node
namespace genode {

    /**
     * This is a general node class used by both the LinkedLIst and Stack.
     */
    template <class T>
    class Node {
    private:
        // The actual data being stored. Note that this is not a pointer.
        T value;
        // The pointer to the next node in the list.
        Node<T>* nextNode;

    public:
        // Constructors and Deconstructors.
        Node(T value);
        Node(Node<T>* firstNode, T value);
        ~Node();

    public:
        // Getters and setters.
        void setValue(T value);
        T getValue() const;
        // Getter with a reference to the value.
        T& getRefValue();
        // Getters and setters for the previous node.
        Node<T>* getNextNode() const;
        void setNextNode(Node<T>* nextNode);
    };

    /**
     * The constructor for the Node class.
     *
     * @param value The value of the node. (Pass in the data via pass by value)
     */
    template<class T> Node<T>::Node(T value) {
        // The value is copied in the constructor and is copied again into the value.
        this->value = value;
        // Initalize the next node to a null pointer.
        this->nextNode = nullptr;
    }

    /**
     * The constructor for the Node class.
     *
     * @param nextNode The pointer to the next node.
     * @param value The value of the node. (Pass by value.)
     */
    template<class T> Node<T>::Node(Node<T>* nextNode, T value) {
        this->value = value;
        this->nextNode = nextNode;
    }

    /**
     * The deconstructor for the Node class.
     */
    template<class T> Node<T>::~Node() {}

    /**
     * Set the value of the Node.
     *
     * @param value The value to set.
     */
    template<class T> void Node<T>::setValue(T value) {
        this->value = value;
    }

    /**
     * Get the value of the Node.
     *
     * @returns The value of the Node.
     */
    template<class T> T Node<T>::getValue() const {
        return this->value;
    }

    /**
     * Get the reference to the value of the node.
     *
     * @returns The reference to the node value. (Modification permitted.)
     */
    template<class T> T& Node<T>::getRefValue() {
        return this->value;
    }

    /**
     * Set the pointer to the next node.
     *
     * @param nextNode The pointer to the next node.
     */
    template<class T> void Node<T>::setNextNode(Node<T>* nextNode) {
        this->nextNode = nextNode;
    }

    /**
     * Get the pointer to the next node. (Constant Method)
     *
     * @returns The pointer to the next node.
     */
    template<class T> Node<T>* Node<T>::getNextNode() const {
        return this->nextNode;
    }
}
#endif