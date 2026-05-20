#ifndef __CIRCULARLINKEDLIST_H__
#define __CIRCULARLINKEDLIST_H__

#include "linkedlist.h"

// Iterador: detecta vuelta completa comparando con m_pRoot
template <typename Container>
class CLLForwardIterator
    : public general_iterator<Container, CLLForwardIterator<Container>> {
    using Node = typename Container::Node;
    Node* m_pRoot;
public:
    using MySelf = CLLForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    CLLForwardIterator(Container* c, Node* n, Node* root)
        : Parent(c, n), m_pRoot(root) {}
    MySelf& operator++() {
        if (this->m_pNode) {
            Node* nx = this->m_pNode->getNext();
            this->m_pNode = (nx == m_pRoot) ? nullptr : nx;
        }
        return *this;
    }
};

template <typename Trait>
class CircularLinkedList : public LinkedList<Trait> {
public:
    using value_type       = typename Trait::value_type;
    using Node             = typename Trait::Node;
    using MySelf           = CircularLinkedList<Trait>;
    using circular_iterator = CLLForwardIterator<MySelf>;
    friend circular_iterator;

    CircularLinkedList() : LinkedList<Trait>() {}
    virtual ~CircularLinkedList() { clear(); }

    circular_iterator cbegin() const {
        return circular_iterator(const_cast<MySelf*>(this), this->m_pRoot, this->m_pRoot);
    }
    circular_iterator cend() const {
        return circular_iterator(const_cast<MySelf*>(this), nullptr, this->m_pRoot);
    }

    void push_back(value_type v, Ref r) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* n = new Node(v, r);
        if (!this->m_size) { this->m_pRoot = this->m_tail = n; n->setNext(n); }
        else               { n->setNext(this->m_pRoot); this->m_tail->setNext(n); this->m_tail = n; }
        this->m_size++;
    }
    void push_front(value_type v, Ref r) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* n = new Node(v, r);
        if (!this->m_size) { this->m_pRoot = this->m_tail = n; n->setNext(n); }
        else               { n->setNext(this->m_pRoot); this->m_tail->setNext(n); this->m_pRoot = n; }
        this->m_size++;
    }
    tuple<value_type, Ref> pop_front() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("lista vacia");
        Node* t  = this->m_pRoot;
        auto  rv = make_tuple(t->getData(), t->getRef());
        if (this->m_size == 1) { this->m_pRoot = this->m_tail = nullptr; }
        else { this->m_pRoot = t->getNext(); this->m_tail->setNext(this->m_pRoot); }
        delete t; this->m_size--;
        return rv;
    }
    tuple<value_type, Ref> pop_back() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("lista vacia");
        auto rv = make_tuple(this->m_tail->getData(), this->m_tail->getRef());
        if (this->m_size == 1) { delete this->m_tail; this->m_pRoot = this->m_tail = nullptr; }
        else {
            Node* cur = this->m_pRoot;
            while (cur->getNext() != this->m_tail) cur = cur->getNext();
            delete this->m_tail; this->m_tail = cur; this->m_tail->setNext(this->m_pRoot);
        }
        this->m_size--;
        return rv;
    }
    void insert(const value_type& v, Ref r) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* n = new Node(v, r);
        if (!this->m_size) { this->m_pRoot = this->m_tail = n; n->setNext(n); }
        else if (this->m_comp(v, this->m_pRoot->getDataRef())) {
            n->setNext(this->m_pRoot); this->m_tail->setNext(n); this->m_pRoot = n;
        } else {
            Node* cur = this->m_pRoot;
            while (cur->getNext() != this->m_pRoot && !this->m_comp(v, cur->getNext()->getDataRef()))
                cur = cur->getNext();
            n->setNext(cur->getNext()); cur->setNext(n);
            if (cur == this->m_tail) this->m_tail = n;
        }
        this->m_size++;
    }
    void clear() override { this->internal_clear_circular(); }

    template <typename Func, typename... Args>
    void circularForEach(size_t vueltas, Func func, Args&&... args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot || !vueltas) return;
        Node* cur = this->m_pRoot;
        for (size_t i = 0; i < this->m_size * vueltas; ++i, cur = cur->getNext())
            func(cur->getDataRef(), forward<Args>(args)...);
    }
};

#endif // __CIRCULARLINKEDLIST_H__