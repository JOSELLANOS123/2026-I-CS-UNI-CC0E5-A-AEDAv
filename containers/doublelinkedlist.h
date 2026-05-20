#ifndef __DOUBLELINKEDLIST_H__
#define __DOUBLELINKEDLIST_H__

#include "linkedlist.h"

// ─── Nodo ─────────────────────────────────────────────────────────────────────
template <typename T>
class DLLNode : public LLNode<T, DLLNode<T>> {
    using Base = LLNode<T, DLLNode<T>>;
    DLLNode* m_prev;
public:
    using value_type = T;
    DLLNode() : Base(), m_prev(nullptr) {}
    DLLNode(T data, Ref ref, DLLNode* next = nullptr, DLLNode* prev = nullptr)
        : Base(data, ref, next), m_prev(prev) {}
    virtual ~DLLNode() {}
    DLLNode*  getPrev() const    { return m_prev; }
    DLLNode*& getPrevRef()       { return m_prev; }
    void      setPrev(DLLNode* p){ m_prev = p; }
};


// ─── Iterador backward ────────────────────────────────────────────────────────
template <typename Container>
class DLLBackwardIterator
    : public general_iterator<Container, DLLBackwardIterator<Container>> {
public:
    using MySelf = DLLBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    MySelf& operator++() {
        if (this->m_pNode) this->m_pNode = this->m_pNode->getPrev();
        return *this;
    }
};

// ─── DoubleLinkedList ─────────────────────────────────────────────────────────
template <typename Trait>
class DoubleLinkedList : public LinkedList<Trait> {
public:
    using value_type       = typename Trait::value_type;
    using Node             = typename Trait::Node;
    using MySelf           = DoubleLinkedList<Trait>;
    using backward_iterator = DLLBackwardIterator<MySelf>;
    friend backward_iterator;

    DoubleLinkedList() : LinkedList<Trait>() {}

public:
    void push_front(value_type v, Ref r) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* n = new Node(v, r, this->m_pRoot, nullptr);
        if (this->m_pRoot) this->m_pRoot->setPrev(n);
        else               this->m_tail = n;
        this->m_pRoot = n;
        this->m_size++;
    }
    void push_back(value_type v, Ref r) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* n = new Node(v, r, nullptr, this->m_tail);
        if (this->m_tail) this->m_tail->setNext(n);
        else              this->m_pRoot = n;
        this->m_tail = n;
        this->m_size++;
    }
    tuple<value_type, Ref> pop_front() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("lista vacia");
        Node* t  = this->m_pRoot;
        auto  rv = make_tuple(t->getData(), t->getRef());
        this->m_pRoot = t->getNext();
        if (this->m_pRoot) this->m_pRoot->setPrev(nullptr);
        else               this->m_tail = nullptr;
        delete t; this->m_size--;
        return rv;
    }
    tuple<value_type, Ref> pop_back() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("lista vacia");
        Node* t  = this->m_tail;
        auto  rv = make_tuple(t->getData(), t->getRef());
        this->m_tail = t->getPrev();
        if (this->m_tail) this->m_tail->setNext(nullptr);
        else              this->m_pRoot = nullptr;
        delete t; this->m_size--;
        return rv;
    }
    void insert(const value_type& v, Ref r) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        internal_insert_dll(this->m_pRoot, nullptr, v, r);
    }

    backward_iterator rbegin() { return backward_iterator(this, this->m_tail); }
    backward_iterator rend()   { return backward_iterator(this, nullptr); }

    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args&&... args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        for (auto it = rbegin(); it != rend(); ++it)
            func(*it, forward<Args>(args)...);
    }

private:
    // insert ordenado con punteros prev correctos
    void internal_insert_dll(Node*& cur, Node* prev, const value_type& v, Ref r) {
        if (!cur || this->m_comp(v, cur->getDataRef())) {
            Node* n = new Node(v, r, cur, prev);
            if (cur)  cur->setPrev(n);
            else      this->m_tail = n;
            cur = n;
            this->m_size++;
            return;
        }
        internal_insert_dll(cur->getNextRef(), cur, v, r);
    }
};

#endif // __DOUBLELINKEDLIST_H__