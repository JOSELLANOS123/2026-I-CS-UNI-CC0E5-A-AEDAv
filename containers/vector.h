#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <iostream>
#include <cstddef> // size_t
#include <string>
#include <sstream>
// #include "../GeneralIterator.h"
#include "util.h"
#include "../types.h"
using namespace std;

template <typename Container>
class vector_forward_iterator{
public:
    typedef typename Container::value_type    value_type;
    typedef typename Container::Node          Node;
    typedef vector_forward_iterator<Container> myself;
protected:
    Container *m_pContainer;
    size_t     m_pos;
public:
    vector_forward_iterator(Container *pContainer, size_t pos)
        : m_pContainer(pContainer), m_pos(pos) {}
    vector_forward_iterator(myself &other) 
          : m_pContainer(other.m_pContainer), m_pos(other.m_pos){}
    vector_forward_iterator(myself &&other) // Move constructor
          {   m_pContainer = move(other.m_pContainer);
              m_pos        = move(other.m_pos);
          }
    myself operator=(myself &iter)
          {   m_pContainer = move(iter.m_pContainer);
              m_pos        = move(iter.m_pos);
              return *(myself *)this; // Pending static_cast?
          }

    bool operator==(myself iter)   { return !(*this != iter); }
    bool operator!=(myself iter)   { return m_pContainer != iter.m_pContainer || m_pos != iter.m_pos; }
    value_type &operator*()        { return m_pContainer->m_data[m_pos].getDataRef();   }
    myself operator++()            { m_pos++; return *this; }
};

template <typename T>
class VectorNode{
    T   m_data;
    Ref m_ref;
public:
    VectorNode() : m_data(T()), m_ref(Ref()) {}
    VectorNode(T data, Ref ref) : m_data(data), m_ref(ref) {}
    VectorNode(const VectorNode &other) : m_data(other.m_data), m_ref(other.m_ref) {}
    VectorNode(VectorNode &&other) : m_data(move(other.m_data)), m_ref(move(other.m_ref)) {}
    VectorNode& operator=(const VectorNode &other) {
        m_data = other.m_data;
        m_ref = other.m_ref;
        return *this;
    }
    VectorNode& operator=(VectorNode &&other) {
        m_data = move(other.m_data);
        m_ref = move(other.m_ref);
        return *this;
    }

    T    getData() const { return m_data; }
    T&   getDataRef() { return m_data; }
    void setData(T data) { m_data = data; }
    Ref  getRef() { return m_ref; }
    void setRef(Ref ref) { m_ref = ref; }
    
    string toString(){
        ostringstream oss;
        oss << "(" << m_data << ", " << m_ref << ")";
        return oss.str();
    }
};

template <typename T>
ostream& operator<<(ostream& os, VectorNode<T>& node){
    return os << node.toString();
}

template <typename T>
class Vector{
public:
    using value_type = T;
    using forward_iterator   = vector_forward_iterator< Vector<T> > ;
    using Node               = VectorNode<T>;
    friend forward_iterator;
private:
    size_t  m_capacity;
    size_t  m_size;
    Node   *m_data;
    void resize();
public:
    Vector(size_t capacity = 10);
    virtual ~Vector();
    virtual void push_back(value_type value, Ref ref);
    virtual value_type  get(size_t index);
    virtual size_t  size();
    virtual string toString();

    forward_iterator begin() { return forward_iterator(this, 0); }
    forward_iterator end()   { return forward_iterator(this, m_size); }

    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...  args){
        ::ForEach(begin(), end(), func, std::forward<Args>(args)... );
    }
};

template <typename T>
Vector<T>::Vector(size_t capacity){
    m_capacity = capacity;
    m_size = 0;
    m_data = new Node[capacity];
}

template <typename T>
Vector<T>::~Vector(){
    delete [] m_data;
}

template <typename T>
void Vector<T>::resize(){
    m_capacity = (m_capacity < 10) ? m_capacity+10 : m_capacity * 2;
    Node * new_data = new Node[m_capacity];
    for(size_t i = 0; i < m_size; ++i)
        new_data[i] = m_data[i];
    delete [] m_data;
    m_data = new_data;
}

template <typename T>
void Vector<T>::push_back(value_type value, Ref ref){
    if(m_size == m_capacity) // Overflow
        resize();
    m_data[m_size++] = Node(value, ref);
}

template <typename T>
typename Vector<T>::value_type
Vector<T>::get(size_t index){
    if(index >= 0 && index < m_size)
        return m_data[index].getData();
    throw std::out_of_range("Index out of range");
}

template <typename T>
size_t Vector<T>::size(){
    return m_size;
}

template <typename T>
string Vector<T>::toString(){
    ostringstream oss;
    oss << "[";
    for(size_t i = 0; i < m_size - 1; ++i)
        oss << m_data[i] << ",";
    if(m_size > 0)
        oss << m_data[m_size - 1];
    oss << "]";
    return oss.str();
}

template <typename T>
ostream& operator<<(ostream& os, Vector<T>& v){
    return os << v.toString();
}

// TODO: Implementar como PR
template <typename T>
istream& operator>>(istream& is, Vector<T>& v){
    return is;
}

// template <typename T>
// template <typename Func, typename... Args>
// void Vector<T>::ForEach(Func func, Args &&...  args){
//     ::ForEach(begin(), end(), func, std::forward<Args>(args)... );
// }

void DemoVector();

#endif // __VECTOR_H__
