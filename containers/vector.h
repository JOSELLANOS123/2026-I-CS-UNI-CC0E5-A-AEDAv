#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <cstddef> // size_t
#include "../types.h"

template <typename T>
class Vector{
    using value_type = T;
private:
    size_t  m_capacity;
    size_t  m_size;
    T * m_data;
public:
    Vector(size_t capacity);
    virtual ~Vector();
    virtual void push_back(T value);
    virtual T  get(size_t index);
    virtual size_t  size();
};

template <typename T>
Vector<T>::Vector(size_t capacity){
    m_capacity = capacity;
    m_size = 0;
    m_data = new T[capacity];
}

template <typename T>
Vector<T>::~Vector(){
    delete[] m_data;
}

template <typename T>
void Vector<T>::push_back(T value){
    if(m_size < m_capacity)
        m_data[m_size++] = value;
}

template <typename T>
T Vector<T>::get(size_t index){
    if(index >= 0 && index < m_size)
        return m_data[index];
    return -1;
}

template <typename T>
size_t Vector<T>::size(){
    return m_size;
}

void DemoVector();

#endif // __VECTOR_H__
