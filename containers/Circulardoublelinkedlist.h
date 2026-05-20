#ifndef __CIRCULARDOUBLELINKEDLIST_H__
#define __CIRCULARDOUBLELINKEDLIST_H__

#include "doublelinkedlist.h"

template <typename Container>
class CDLLForwardIterator
    : public general_iterator<Container, CDLLForwardIterator<Container>> {
    using Node = typename Container::Node;
    Node* m_pRoot;
public:
    using MySelf = CDLLForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    CDLLForwardIterator(Container* c, Node* n, Node* root) : Parent(c, n), m_pRoot(root) {}
    MySelf& operator++() {
        if (this->m_pNode) {
            Node* nx = this->m_pNode->getNext();
            this->m_pNode = (nx == m_pRoot) ? nullptr : nx;
        }
        return *this;
    }
};

template <typename Container>
class CDLLBackwardIterator
    : public general_iterator<Container, CDLLBackwardIterator<Container>> {
    using Node = typename Container::Node;
    Node* m_pTail;
public:
    using MySelf = CDLLBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    CDLLBackwardIterator(Container* c, Node* n, Node* tail) : Parent(c, n), m_pTail(tail) {}
    MySelf& operator++() {
        if (this->m_pNode) {
            Node* pv = this->m_pNode->getPrev();
            this->m_pNode = (pv == m_pTail) ? nullptr : pv;
        }
        return *this;
    }
};

template <typename Trait>
class CircularDoubleLinkedList : public DoubleLinkedList<Trait> {
public:
    using value_type        = typename Trait::value_type;
    using Node              = typename Trait::Node;
    using MySelf            = CircularDoubleLinkedList<Trait>;
    using forward_iterator  = CDLLForwardIterator<MySelf>;
    using backward_iterator = CDLLBackwardIterator<MySelf>;
    friend forward_iterator;
    friend backward_iterator;

    CircularDoubleLinkedList() : DoubleLinkedList<Trait>() {}
    virtual ~CircularDoubleLinkedList() { clear(); }

    forward_iterator  cbegin()  const { return forward_iterator (const_cast<MySelf*>(this), this->m_pRoot, this->m_pRoot); }
    forward_iterator  cend()    const { return forward_iterator (const_cast<MySelf*>(this), nullptr,       this->m_pRoot); }
    backward_iterator crbegin() const { return backward_iterator(const_cast<MySelf*>(this), this->m_tail,  this->m_tail);  }
    backward_iterator crend()   const { return backward_iterator(const_cast<MySelf*>(this), nullptr,       this->m_tail);  }

    void push_back(value_type v, Ref r) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* n = new Node(v, r);
        if (!this->m_size) { this->m_pRoot = this->m_tail = n; n->setNext(n); n->setPrev(n); }
        else {
            n->setNext(this->m_pRoot); n->setPrev(this->m_tail);
            this->m_tail->setNext(n); this->m_pRoot->setPrev(n);
            this->m_tail = n;
        }
        this->m_size++;
    }
    void push_front(value_type v, Ref r) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* n = new Node(v, r);
        if (!this->m_size) { this->m_pRoot = this->m_tail = n; n->setNext(n); n->setPrev(n); }
        else {
            n->setNext(this->m_pRoot); n->setPrev(this->m_tail);
            this->m_pRoot->setPrev(n); this->m_tail->setNext(n);
            this->m_pRoot = n;
        }
        this->m_size++;
    }
    tuple<value_type, Ref> pop_front() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("lista vacia");
        Node* t = this->m_pRoot;
        auto  rv = make_tuple(t->getData(), t->getRef());
        if (this->m_size == 1) { this->m_pRoot = this->m_tail = nullptr; }
        else {
            this->m_pRoot = t->getNext();
            this->m_pRoot->setPrev(this->m_tail);
            this->m_tail->setNext(this->m_pRoot);
        }
        delete t; this->m_size--;
        return rv;
    }
    tuple<value_type, Ref> pop_back() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("lista vacia");
        Node* t = this->m_tail;
        auto  rv = make_tuple(t->getData(), t->getRef());
        if (this->m_size == 1) { this->m_pRoot = this->m_tail = nullptr; }
        else {
            this->m_tail = t->getPrev();
            this->m_tail->setNext(this->m_pRoot);
            this->m_pRoot->setPrev(this->m_tail);
        }
        delete t; this->m_size--;
        return rv;
    }
    void insert(const value_type& v, Ref r) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* n = new Node(v, r);
        if (!this->m_size) { this->m_pRoot = this->m_tail = n; n->setNext(n); n->setPrev(n); }
        else if (this->m_comp(v, this->m_pRoot->getDataRef())) {
            n->setNext(this->m_pRoot); n->setPrev(this->m_tail);
            this->m_pRoot->setPrev(n); this->m_tail->setNext(n); this->m_pRoot = n;
        } else {
            Node* cur = this->m_pRoot;
            while (cur->getNext() != this->m_pRoot && !this->m_comp(v, cur->getNext()->getDataRef()))
                cur = cur->getNext();
            Node* nx = cur->getNext();
            n->setNext(nx); n->setPrev(cur);
            cur->setNext(n); nx->setPrev(n);
            if (cur == this->m_tail) this->m_tail = n;
        }
        this->m_size++;
    }
    void clear() override { this->internal_clear_circular(); }
    template <typename Func, typename... Args>
    void circularForEach(size_t vueltas, int dir, Func func, Args&&... args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot || !vueltas) return;
        Node* cur = (dir >= 0) ? this->m_pRoot : this->m_tail;
        for (size_t i = 0; i < this->m_size * vueltas; ++i) {
            func(cur->getDataRef(), forward<Args>(args)...);
            cur = (dir >= 0) ? cur->getNext() : cur->getPrev();
        }
    }
};

#endif // __CIRCULARDOUBLELINKEDLIST_H__