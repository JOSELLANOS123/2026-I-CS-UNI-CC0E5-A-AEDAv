#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <iostream>
#include <cstddef> // size_t
#include <string>
#include <sstream>
#include <shared_mutex> // shared_mutex
#include "general_iterator.h"
#include "util.h"
#include "../types.h"
using namespace std;

// Forward iterator
template <typename Container>
class LinkedListForwardIterator : public general_iterator<Container, LinkedListForwardIterator<Container>>{
    using MySelf = LinkedListForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    // TODO: Completar el operator++
    // T4: forward iterator op++ avanza al siguiente nodo
    MySelf operator++() { this->m_pNode = this->m_pNode->getNext(); return *this; }
};
    

// Linked List Node
template <typename T>
class LLNode{
    using Node = LLNode<T>;
private:
    T   m_data;
    Ref   m_ref;
    Node *m_next;
    
public:
    LLNode()  : m_data(T()), m_ref(Ref()), m_next(nullptr) {}
    LLNode(T data, Ref ref) : m_data(data), m_ref(ref), m_next(nullptr) {}
    LLNode(T data, Ref ref, Node *next) : m_data(data), m_ref(ref), m_next(next) {}
    virtual ~LLNode() {}

    T      getData() const { return m_data; }
    T&     getDataRef()    { return m_data; }
    void   setData(T data) { m_data = data; }
    Ref    getRef()  const { return m_ref; }
    void   setRef(Ref ref) { m_ref = ref; }
    Node*  getNext() const { return m_next; }
    Node*& getNextRef()    { return m_next; }
    void   setNext(Node *next) { m_next = next; }
};

template <typename T>
struct AscendingLinkedListTrait{
    using value_type = T;
    using Node = LLNode<T>;
    using Comp = less<T>;
};

template <typename T>
struct DescendingLinkedListTrait{
    using value_type = T;
    using Node = LLNode<T>;
    using Comp = greater<T>;
};

template <typename Trait>
class LinkedList{
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using MySelf     = LinkedList<Trait>;

    using forward_iterator = LinkedListForwardIterator<MySelf>;
    friend forward_iterator;

private:
    Node *m_pRoot = nullptr;
    Node *m_tail = nullptr;
    size_t m_size = 0;
    Comp   m_comp;
    mutable shared_mutex m_mtx;
    mutable shared_mutex m_mtx;
            void internal_insert(Node* &pPrev, const value_type &value, Ref ref);
public:
    LinkedList() {}
    LinkedList(const LinkedList &other){ // T1: Copy constructor
    }
    LinkedList(LinkedList &&other) : m_pRoot(nullptr), m_tail(nullptr), m_size(0) { // T2: move constructor - roba punteros y deja origen vacio
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = other.m_pRoot;
        m_tail  = other.m_tail;
        m_size  = other.m_size;
        other.m_pRoot = nullptr;
        other.m_tail  = nullptr;
        other.m_size  = 0;
    }
    virtual ~LinkedList(){ // T3: destructor seguro - recorre y libera cada nodo
        unique_lock<shared_mutex> lock(m_mtx);
        while(m_pRoot){
            Node *tmp = m_pRoot;
            m_pRoot = m_pRoot->getNext();
            delete tmp;
        }
    }
   
    
    virtual void    push_front(value_type value, Ref ref);
    virtual void    pop_front();
    virtual void    push_back(value_type value, Ref ref);
    virtual void    pop_back();
    virtual void    insert(const value_type &value, Ref ref);


    virtual value_type& operator[](size_t index);
    virtual size_t      size() const;
    virtual string      toString() const;
    


    forward_iterator begin() { return forward_iterator(this, m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr); }

    
    template <typename Func, typename... Args> // T12: ForEach - itera aplicando funcion a cada elemento
    void ForEach(Func func, Args &&...  args){
        unique_lock<shared_mutex> lock(m_mtx);
        ::ForEach(begin(), end(), func, std::forward<Args>(args)... );
    }
};


template <typename Trait> // T5: push_front - inserta al inicio
void LinkedList<Trait>::push_front(value_type value, Ref ref){
    unique_lock<shared_mutex> lock(m_mtx);
    m_pRoot = new Node(value, ref, m_pRoot);
    if(m_size == 0) m_tail = m_pRoot;
    m_size++;
}


template <typename Trait> // T6: pop_front - elimina al inicio
void LinkedList<Trait>::pop_front(){
    unique_lock<shared_mutex> lock(m_mtx);
    if(!m_pRoot) return;
    Node *tmp = m_pRoot;
    m_pRoot = m_pRoot->getNext();
    if(!m_pRoot) m_tail = nullptr;
    delete tmp;
    m_size--;
}


template <typename Trait> // T7: push_back - inserta al final sin recorrer todo
void LinkedList<Trait>::push_back(value_type value, Ref ref){
    unique_lock<shared_mutex> lock(m_mtx);
    Node *node = new Node(value, ref);
    if(!m_tail){
        m_pRoot = m_tail = node;
    } else {
        m_tail->setNext(node);
        m_tail = node;
    }
    m_size++;
}

template <typename Trait> // T8: pop_back - elimina al final, recorre hasta penultimo
void LinkedList<Trait>::pop_back(){
    unique_lock<shared_mutex> lock(m_mtx);
    if(!m_pRoot) return;
    if(m_pRoot == m_tail){
        delete m_pRoot;
        m_pRoot = m_tail = nullptr;
    } else {
        Node *p = m_pRoot;
        while(p->getNext() != m_tail)
            p = p->getNext();
        delete m_tail;
        m_tail = p;
        m_tail->setNext(nullptr);
    }
    m_size--;
}


template <typename Trait>
void LinkedList<T>::internal_insert(Node* &pPr  ev, const value_type &value, Ref ref){
    if(!pPrev || m_comp(value, pPrev->getDataRef())){
        pPrev = new Node(value, ref, pPrev);
        m_size++;
          if(!pPrev->getNext())
                 m_tail = pPrev;
        return;
    }
    internal_insert(pPrev->getNextRef(), value, ref);
}

template <typename Trait>
void LinkedList<Trait>::insert(const value_type &value, Ref ref){
    unique_lock<shared_mutex> lock(m_mtx);
    internal_insert(m_pRoot, value, ref);
}


template <typename Trait> // T9: operator[] - accede a la posicion requerida
typename LinkedList<Trait>::value_type& LinkedList<Trait>::operator[](size_t index){
    shared_lock<shared_mutex> lock(m_mtx);
    Node *p = m_pRoot;
    for(size_t i = 0; i < index; i++)
        p = p->getNext();
    return p->getDataRef();
}

template <typename Trait>
size_t LinkedList<Trait>::size() const{
    shared_lock<shared_mutex> lock(m_mtx);
    return m_size;
}


template <typename Trait> // T11: toString - imprime lista con formato [(data, ref), ...]
string LinkedList<Trait>::toString() const{
    shared_lock<shared_mutex> lock(m_mtx);
    ostringstream oss;
    oss << "[";
    Node *p = m_pRoot;
    while(p){
        oss << "(" << p->getData() << ", " << p->getRef() << ")";
        if(p->getNext()) oss << ",";
        p = p->getNext();
    }
    oss << "]";
    return oss.str();
}

template <typename Trait>
ostream& operator<<(ostream& os, const LinkedList<Trait>& list){
    return os << list.toString();
}
 
// T10: operator>> - TODO
template <typename Trait>
istream& operator>>(istream& is, LinkedList<Trait>& list){
    return is;
}


#endif // __LINKEDLIST_H__