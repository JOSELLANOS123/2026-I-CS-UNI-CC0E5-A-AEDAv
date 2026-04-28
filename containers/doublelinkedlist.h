#ifndef __DOUBLELINKEDLIST_H__
#define __DOUBLELINKEDLIST_H__

#include "linkedlist.h"

// TODO Los iteradores ahora son forward y backward
// Crear 2 nuevos i
template <typename T>
class DLLNode : public LLNode<T, DLLNode<T>>{
    using Node = DLLNode<T>;
    private:
        Node *m_pPrev;
    public:
        DLLNode() : LLNode<T, DLLNode<T>>(), m_pPrev(nullptr) {}
        DLLNode(T data, Ref ref, Node *next = nullptr, Node *prev = nullptr) : LLNode<T, DLLNode<T>>(data, ref, next), m_pPrev(prev) {}
        virtual ~DLLNode() {}
        Node*  getPrev() const     { return m_pPrev; }
        void   setPrev(Node *prev) { m_pPrev = prev; }
        Node*& getPrevRef()        { return m_pPrev; }

};

template <typename T>
struct AscendingDLLTrait : BaseTrait<T, less<T>, DLLNode<T>>{};
template <typename T>
struct DescendingDLLTrait : BaseTrait<T, greater<T>, DLLNode<T>> {};

template <typename Container> //  Iterador Forward — avanza con getNext()
class DLLForwardIterator
    : public general_iterator<Container, DLLForwardIterator<Container>> {
public:
    using MySelf = DLLForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
 
    MySelf operator++() {
        if (this->m_pNode)
            this->m_pNode = this->m_pNode->getNext();
        return *this;
    }
};

template <typename Container> //  Iterador Backward — avanza con getPrev() (recorre hacia atras)
class DLLBackwardIterator
    : public general_iterator<Container, DLLBackwardIterator<Container>> {
public:
    using MySelf = DLLBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
 
    MySelf operator++() {
        if (this->m_pNode)
            this->m_pNode = this->m_pNode->getPrev();
        return *this;
    }
};


template <typename Trait>
class DoubleLinkedList : public LinkedList<Trait> {
public:
    using value_type       = typename Trait::value_type;
    using Node             = typename Trait::Node;
    using Comp             = typename Trait::Comp;
    using MySelf           = DoubleLinkedList<Trait>;
    using forward_iterator  = DLLForwardIterator<MySelf>;
    using backward_iterator = DLLBackwardIterator<MySelf>;
    friend forward_iterator;
    friend backward_iterator;
 
private:
    Node  *m_pRoot = nullptr;
    Node  *m_tail  = nullptr;
    size_t m_size  = 0;
    Comp   m_comp;
    mutable shared_mutex m_mtx;
 
    // internal_insert: inserta ordenado y actualiza m_pPrev del nodo siguiente
    void internal_insert(Node *&actual, Node *pPrev, const value_type &value, Ref ref) {
        if (!actual || m_comp(value, actual->getDataRef())) {
            Node *newNode = new Node(value, ref, actual, pPrev);
            if (actual)
                actual->setPrev(newNode);   // el siguiente apunta hacia atras al nuevo
            actual = newNode;
            m_size++;
            if (newNode->getNext() == nullptr)
                m_tail = newNode;
            return;
        }
        internal_insert(actual->getNextRef(), actual, value, ref);
    }
 
public:
    DoubleLinkedList() : LinkedList<Trait>() {}
 
    // Copy constructor: recorre el original con shared_lock y copia nodo a nodo
    DoubleLinkedList(const DoubleLinkedList &other)
        : LinkedList<Trait>(), m_pRoot(nullptr), m_tail(nullptr), m_size(0) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        for (Node *c = other.m_pRoot; c != nullptr; c = c->getNext())
            push_back(c->getData(), c->getRef());
    }
 
    // Move constructor: usa std::exchange para vaciado atomico del original
    DoubleLinkedList(DoubleLinkedList &&other)
        : LinkedList<Trait>(), m_pRoot(nullptr), m_tail(nullptr), m_size(0) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = std::exchange(other.m_pRoot, nullptr);
        m_tail  = std::exchange(other.m_tail,  nullptr);
        m_size  = std::exchange(other.m_size,  0);
    }
 
    // Copy assignment
    DoubleLinkedList &operator=(const DoubleLinkedList &other) {
        if (this != &other) {
            clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            for (Node *c = other.m_pRoot; c != nullptr; c = c->getNext())
                push_back(c->getData(), c->getRef());
        }
        return *this;
    }
 
    // Move assignment
    DoubleLinkedList &operator=(DoubleLinkedList &&other) {
        if (this != &other) {
            clear();
            unique_lock<shared_mutex> lock(other.m_mtx);
            m_pRoot = std::exchange(other.m_pRoot, nullptr);
            m_tail  = std::exchange(other.m_tail,  nullptr);
            m_size  = std::exchange(other.m_size,  0);
        }
        return *this;
    }
 
    virtual ~DoubleLinkedList() { clear(); }
 
    void clear() {
        unique_lock<shared_mutex> lock(m_mtx);
        Node *act = m_pRoot;
        while (act) {
            Node *next = act->getNext();
            delete act;
            act = next;
        }
        m_pRoot = nullptr;
        m_tail  = nullptr;
        m_size  = 0;
    }
 
    // sobreescriben LinkedList para mantener m_pPrev 
 
    void push_front(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(m_mtx);
        Node *newNode = new Node(value, ref, m_pRoot, nullptr);
        if (m_pRoot) m_pRoot->setPrev(newNode);
        else         m_tail = newNode;
        m_pRoot = newNode;
        m_size++;
    }
 
    void push_back(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(m_mtx);
        Node *newNode = new Node(value, ref, nullptr, m_tail);
        if (m_tail) m_tail->setNext(newNode);
        else        m_pRoot = newNode;
        m_tail = newNode;
        m_size++;
    }
 
    void insert(const value_type &value, Ref ref) override {
        unique_lock<shared_mutex> lock(m_mtx);
        internal_insert(m_pRoot, nullptr, value, ref);
        if (m_size == 1) m_tail = m_pRoot;
    }
 
    std::tuple<value_type, Ref> pop_front() override {
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) throw runtime_error("La lista esta vacia");
        Node *temp   = m_pRoot;
        auto  result = std::make_tuple(temp->getData(), temp->getRef());
        m_pRoot      = temp->getNext();
        if (m_pRoot) m_pRoot->setPrev(nullptr);
        else         m_tail = nullptr;
        delete temp;
        m_size--;
        return result;
    }
 
    std::tuple<value_type, Ref> pop_back() override {
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) throw runtime_error("La lista esta vacia");
        Node *temp   = m_tail;
        auto  result = std::make_tuple(temp->getData(), temp->getRef());
        m_tail       = temp->getPrev();
        if (m_tail) m_tail->setNext(nullptr);
        else        m_pRoot = nullptr;
        delete temp;
        m_size--;
        return result;
    }
 
    value_type &operator[](size_t index) override {
        shared_lock<shared_mutex> lock(m_mtx);
        if (index >= m_size) throw out_of_range("Indice fuera de rango");
        Node *act = m_pRoot;
        for (size_t i = 0; i < index; ++i) act = act->getNext();
        return act->getDataRef();
    }
 
    size_t size() const override {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_size;
    }
 
    // begin/end (forward) y rbegin/rend (backward) 
    forward_iterator  begin()  { return forward_iterator (this, m_pRoot); }
    forward_iterator  end()    { return forward_iterator (this, nullptr);  }
    backward_iterator rbegin() { return backward_iterator(this, m_tail);   }
    backward_iterator rend()   { return backward_iterator(this, nullptr);  }
 
    // ForEach: lock + delega al ForEach libre de util.h 
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...args) {
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_size == 0) return;
        ::ForEach(begin(), end(), func, std::forward<Args>(args)...);
    }
 
    // ReverseForEach: recorre con backward iterator 
    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args &&...args) {
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_size == 0) return;
        ::ForEach(rbegin(), rend(), func, std::forward<Args>(args)...);
    }
 
    // Mejora libre #1: contains: Verifica si un valor existe en la lista en O(n)
    bool contains(const value_type &value) {
        shared_lock<shared_mutex> lock(m_mtx);
        for (Node *act = m_pRoot; act != nullptr; act = act->getNext())
            if (act->getDataRef() == value) return true;
        return false;
    }
 
    // Mejora libre #2: printBackward: Imprime la lista de cola a cabeza usando el enlace m_pPrev
    void printBackward(ostream &os) const {
        shared_lock<shared_mutex> lock(m_mtx);
        os << "[bwd:";
        for (Node *act = m_tail; act != nullptr; act = act->getPrev()) {
            os << "(" << act->getData() << "," << act->getRef() << ")";
            if (act->getPrev()) os << ",";
        }
        os << "]";
    }
 
    // operator<< : imprime forward; usa printBackward para mostrar enlace prev 
    friend ostream &operator<<(ostream &os, const DoubleLinkedList &list) {
        shared_lock<shared_mutex> lock(list.m_mtx);
        os << "[";
        Node *act = list.m_pRoot;
        while (act) {
            os << "(" << act->getData() << "," << act->getRef() << ")";
            if (act->getNext()) os << ",";
            act = act->getNext();
        }
        os << "]";
        return os;
    }
 
    // operator>> : lee desde stream en formato [(v,r),(v,r),...] 
    friend istream &operator>>(istream &is, DoubleLinkedList &list) {
        char ch;
        if (!(is >> ch) || ch != '[') {
            is.clear(ios_base::failbit);
            return is;
        }
        value_type val;
        Ref        ref;
        char       comma, parenClose;
        while (is >> ch && ch != ']')
            if (ch == '(')
                if (is >> val >> comma >> ref >> parenClose)
                    if (comma == ',' && parenClose == ')')
                        list.insert(val, ref);
        is.ignore(numeric_limits<streamsize>::max(), '\n');
        return is;
    }
};
 
#endif // __DOUBLELINKEDLIST_H__