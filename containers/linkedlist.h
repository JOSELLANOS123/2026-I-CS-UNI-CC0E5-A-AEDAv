#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include <tuple>
#include <type_traits>
#include "general_iterator.h"
#include "util.h"
#include "../types.h"
#include "traits.h"
using namespace std;

// ─── Iterador ─────────────────────────────────────────────────────────────────
template <typename Container>
class LinkedListForwardIterator
    : public general_iterator<Container, LinkedListForwardIterator<Container>> {
public:
    using MySelf = LinkedListForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    MySelf& operator++() {
        if (this->m_pNode) this->m_pNode = this->m_pNode->getNext();
        return *this;
    }
};

// ─── Nodo ─────────────────────────────────────────────────────────────────────
// Error corregido: antes LLNode<T> usaba NodeType sin declararlo como parámetro
// y getNext() aparecía como ngetNext() (typo)
template <typename T, typename Derived = void>
class LLNode {
public:
    using value_type = T;
    using Node = conditional_t<is_void_v<Derived>, LLNode, Derived>;
protected:
    T     m_data;
    Ref   m_ref;
    Node* m_next;
public:
    LLNode() : m_data(T()), m_ref(Ref()), m_next(nullptr) {}
    LLNode(T data, Ref ref, Node* next = nullptr) : m_data(data), m_ref(ref), m_next(next) {}
    virtual ~LLNode() {}

    T      getData()  const { return m_data; }
    T&     getDataRef()     { return m_data; }
    void   setData(T d)     { m_data = d; }
    Ref    getRef()   const { return m_ref; }
    void   setRef(Ref r)    { m_ref = r; }
    Node*  getNext()  const { return m_next; }   // fix: era ngetNext()
    Node*& getNextRef()     { return m_next; }
    void   setNext(Node* n) { m_next = n; }
};

// ─── Traits de linked list ────────────────────────────────────────────────────

// ─── LinkedList ───────────────────────────────────────────────────────────────
template <typename Trait>
class LinkedList {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using MySelf     = LinkedList<Trait>;
    using forward_iterator = LinkedListForwardIterator<MySelf>;
    friend forward_iterator;

protected:
    Node*  m_pRoot = nullptr;
    Node*  m_tail  = nullptr;
    size_t m_size  = 0;
    Comp   m_comp;
    mutable shared_mutex m_mtx;

    virtual void internal_insert(Node*& cur, const value_type& v, Ref r) {
        if (!cur || m_comp(v, cur->getDataRef())) {
            cur = new Node(v, r, cur);
            m_size++;
            if (!cur->getNext()) m_tail = cur;
            return;
        }
        internal_insert(cur->getNextRef(), v, r);
    }

public:
    LinkedList() {}

    LinkedList(const LinkedList& o) : m_pRoot(nullptr), m_tail(nullptr), m_size(0) {
        shared_lock<shared_mutex> lock(o.m_mtx);
        for (Node* c = o.m_pRoot; c; c = c->getNext()) push_back(c->getData(), c->getRef());
    }
    LinkedList(LinkedList&& o) : m_pRoot(nullptr), m_tail(nullptr), m_size(0) {
        unique_lock<shared_mutex> lock(o.m_mtx);
        m_pRoot = exchange(o.m_pRoot, nullptr);
        m_tail  = exchange(o.m_tail,  nullptr);
        m_size  = exchange(o.m_size,  0);
    }
    LinkedList& operator=(const LinkedList& o) {
        if (this != &o) {
            clear();
            shared_lock<shared_mutex> lock(o.m_mtx);
            for (Node* c = o.m_pRoot; c; c = c->getNext()) push_back(c->getData(), c->getRef());
        }
        return *this;
    }
    LinkedList& operator=(LinkedList&& o) {
        if (this != &o) {
            clear();
            unique_lock<shared_mutex> lock(o.m_mtx);
            m_pRoot = exchange(o.m_pRoot, nullptr);
            m_tail  = exchange(o.m_tail,  nullptr);
            m_size  = exchange(o.m_size,  0);
        }
        return *this;
    }
    virtual ~LinkedList() { clear(); }

    virtual void clear() {
        unique_lock<shared_mutex> lock(m_mtx);
        Node* cur = m_pRoot;
        while (cur) { Node* nx = cur->getNext(); delete cur; cur = nx; }
        m_pRoot = m_tail = nullptr; m_size = 0;
    }

    virtual void insert(const value_type& v, Ref r) {
        unique_lock<shared_mutex> lock(m_mtx);
        internal_insert(m_pRoot, v, r);
        if (m_size == 1) m_tail = m_pRoot;
    }
    virtual void push_front(value_type v, Ref r) {
        unique_lock<shared_mutex> lock(m_mtx);
        m_pRoot = new Node(v, r, m_pRoot);
        if (m_size == 0) m_tail = m_pRoot;
        m_size++;
    }
    virtual void push_back(value_type v, Ref r) {
        unique_lock<shared_mutex> lock(m_mtx);
        Node* n = new Node(v, r);
        if (!m_size) { m_pRoot = m_tail = n; }
        else         { m_tail->setNext(n); m_tail = n; }
        m_size++;
    }
    virtual tuple<value_type, Ref> pop_front() {
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) throw runtime_error("lista vacia");
        Node* t  = m_pRoot;
        auto  rv = make_tuple(t->getData(), t->getRef());
        m_pRoot  = t->getNext();
        delete t; m_size--;
        if (!m_size) m_tail = nullptr;
        return rv;
    }
    virtual tuple<value_type, Ref> pop_back() {
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) throw runtime_error("lista vacia");
        auto rv = make_tuple(m_tail->getData(), m_tail->getRef());
        if (m_pRoot == m_tail) { delete m_pRoot; m_pRoot = m_tail = nullptr; }
        else {
            Node* cur = m_pRoot;
            while (cur->getNext() != m_tail) cur = cur->getNext();
            delete m_tail; m_tail = cur; m_tail->setNext(nullptr);
        }
        m_size--;
        return rv;
    }
    virtual value_type& operator[](size_t idx) {
        shared_lock<shared_mutex> lock(m_mtx);
        if (idx >= m_size) throw out_of_range("indice fuera de rango");
        Node* cur = m_pRoot;
        for (size_t i = 0; i < idx; ++i) cur = cur->getNext();
        return cur->getDataRef();
    }
    virtual size_t size() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_size;
    }

    // begin/end usan m_size en operator<< para funcionar también en listas circulares
    virtual forward_iterator begin() const { return forward_iterator(const_cast<MySelf*>(this), m_pRoot); }
    virtual forward_iterator end()   const { return forward_iterator(const_cast<MySelf*>(this), nullptr);  }

    template <typename Func, typename... Args>
    void ForEach(Func func, Args&&... args) {
        unique_lock<shared_mutex> lock(m_mtx);
        auto it = begin();
        for (size_t i = 0; i < m_size; ++i, ++it) func(*it, forward<Args>(args)...);
    }

    // operator<< y operator>> definidos UNA SOLA VEZ — heredados por DLL, CLL, CDLL
    friend ostream& operator<<(ostream& os, const LinkedList& list) {
        shared_lock<shared_mutex> lock(list.m_mtx);
        os << "[";
        auto it = list.begin();
        for (size_t i = 0; i < list.m_size; ++i, ++it) {
            if (i) os << ",";
            os << "(" << it.getNode()->getData() << "," << it.getNode()->getRef() << ")";
        }
        return os << "]";
    }
    friend istream& operator>>(istream& is, LinkedList& list) {
        char ch;
        if (!(is >> ch) || ch != '[') { is.clear(ios_base::failbit); return is; }
        value_type v; Ref r; char cm, cp;
        while (is >> ch && ch != ']')
            if (ch == '(')
                if (is >> v >> cm >> r >> cp && cm == ',' && cp == ')')
                    list.insert(v, r);
        return is;
    }
};

#endif // __LINKEDLIST_H__