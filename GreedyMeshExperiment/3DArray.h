#pragma once

#ifndef DArray_H
#define DArray_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string.h>

/**

	A psuedo 3D array that only uses a single array for speed.

*/
template <class T>
class ThreeDArray {
private:
	T* arr;
	size_t size;

public:
	ThreeDArray(size_t size);
	~ThreeDArray();

	void zeroOut();
	T at(size_t x, size_t y, size_t z);
	T at(glm::vec3 vec);
	void set(size_t x, size_t y, size_t z, T value);
	void set(glm::vec3, T value);

	size_t length() const;
};

template <class T>
ThreeDArray<T>::ThreeDArray(size_t size) {
	this->size = size;
	this->arr = new T[this->size * this->size * this->size];
}

template <class T>
ThreeDArray<T>::~ThreeDArray() {
	delete[] this->arr;
	this->arr = 0;
	this->size = 0;
}

/**

	Zero out the array. O(n)

	Note: Despite the O(n), it is far faster than manually doing it depending on the operating system and CPU.

*/
template<class T>
void ThreeDArray<T>::zeroOut() {
	memset(this->arr, 0, this->size * this->size * this->size * sizeof(T));
}

/**
	Get the value at x, y, z. O(1)
*/
template <class T>
T ThreeDArray<T>::at(size_t x, size_t y, size_t z) {
	return this->arr[x + y * this->size + z * this->size * this->size];
}

/**
	Get the value at the vector location. O(1)
*/
template <class T>
T ThreeDArray<T>::at(glm::vec3 vec) {
	return this->arr[(int)vec.x + (int)vec.y * this->size + (int)vec.z * this->size * this->size];
}

/**
	Set the value at x, y, z, to the specified value. O(1)
*/
template <class T>
void ThreeDArray<T>::set(size_t x, size_t y, size_t z, T value) {
	this->arr[x + y * this->size + z * this->size * this->size] = value;
}

/**
	Set the value at the vector, to the specified value. O(1)
*/
template <class T>
void ThreeDArray<T>::set(glm::vec3 vec, T value) {
	this->arr[(int)vec.x + (int)vec.y * this->size + (int)vec.z * this->size * this->size] = value;
}

/**
	Get the length of the 3D array. O(1)
*/
template <class T>
size_t ThreeDArray<T>::length() const {
	return this->size;
}

#endif