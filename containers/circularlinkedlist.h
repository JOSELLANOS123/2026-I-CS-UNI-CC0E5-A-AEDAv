#ifndef __CIRCULARLINKEDLIST_H__
#define __CIRCULARLINKEDLIST_H__

#include "linkedlist.h"

//  Reutilizan LLNode<T> del linkedlist y BaseTrait de traits.h

template <typename T>
struct AscendingCLLTrait : BaseTrait<T, less<T>, LLNode<T>> {};

template <typename T>
struct DescendingCLLTrait : BaseTrait<T, greater<T>, LLNode<T>> {};

//  CLLForwardIterator
//  El truco circular: guarda m_start para saber cuando dio
//  la vuelta completa y poner m_pNode en nullptr (= end()).

template <typename Container>
class CLLForwardIterator
    : public general_iterator<Container, CLLForwardIterator<Container>> {
public:
    using MySelf = CLLForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Node   = typename Container::Node;

private:
    Node *m_start;   // nodo inicial para detectar vuelta completa

public:
    // Constructor normal: begin() — guarda el nodo de arranque
    CLLForwardIterator(Container *pContainer, Node *pNode)
        : Parent(pContainer, pNode), m_start(pNode) {}

    // Constructor centinela: end() — m_start = nullptr senala fin
    CLLForwardIterator(Container *pContainer, Node *pNode, bool)
        : Parent(pContainer, pNode), m_start(nullptr) {}

    MySelf operator++() {
        if (this->m_pNode) {
            this->m_pNode = this->m_pNode->getNext();
            if (this->m_pNode == m_start)   // vuelta completa: senalar fin
                this->m_pNode = nullptr;
        }
        return *this;
    }
};


//  CircularLinkedList<Trait>: Hereda de LinkedList y sobreescribe lo necesario para que el enlace sea circular: m_tail->next apunta siempre a m_pRoot.

template <typename Trait>
class CircularLinkedList : public LinkedList<Trait> {
public:
    using value_type       = typename Trait::value_type;
    using Node             = typename Trait::Node;
    using Comp             = typename Trait::Comp;
    using MySelf           = CircularLinkedList<Trait>;
    using forward_iterator = CLLForwardIterator<MySelf>;
    friend forward_iterator;

private:
    Node  *m_pRoot = nullptr;
    Node  *m_tail  = nullptr;
    size_t m_size  = 0;
    Comp   m_comp;
    mutable shared_mutex m_mtx;

    // Insercion ordenada manteniendo el enlace circular
    void internal_insert(const value_type &value, Ref ref) {
        Node *newNode = new Node(value, ref);
        m_size++;
        // Lista vacia: el unico nodo se apunta a si mismo
        if (!m_pRoot) {
            newNode->setNext(newNode);
            m_pRoot = newNode;
            m_tail  = newNode;
            return;
        }
        // El nuevo valor va antes del root actual
        if (m_comp(value, m_pRoot->getDataRef())) {
            newNode->setNext(m_pRoot);
            m_tail->setNext(newNode);   // mantener cierre circular
            m_pRoot = newNode;
            return;
        }
        // Buscar el nodo antes del slot correcto
        Node *act = m_pRoot;
        while (act->getNext() != m_pRoot &&
               !m_comp(value, act->getNext()->getDataRef()))
            act = act->getNext();
        newNode->setNext(act->getNext());
        act->setNext(newNode);
        if (act == m_tail)
            m_tail = newNode;
    }

public:
    CircularLinkedList() : LinkedList<Trait>() {}

    // Copy constructor: recorre con do-while para respetar la circularidad
    CircularLinkedList(const CircularLinkedList &other)
        : LinkedList<Trait>(), m_pRoot(nullptr), m_tail(nullptr), m_size(0) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        if (!other.m_pRoot) return;
        Node *curr = other.m_pRoot;
        do {
            push_back(curr->getData(), curr->getRef());
            curr = curr->getNext();
        } while (curr != other.m_pRoot);
    }

    // Move constructor con std::exchange
    CircularLinkedList(CircularLinkedList &&other)
        : LinkedList<Trait>(), m_pRoot(nullptr), m_tail(nullptr), m_size(0) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = std::exchange(other.m_pRoot, nullptr);
        m_tail  = std::exchange(other.m_tail,  nullptr);
        m_size  = std::exchange(other.m_size,  0);
    }

    CircularLinkedList &operator=(const CircularLinkedList &other) {
        if (this != &other) {
            clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            if (!other.m_pRoot) return *this;
            Node *curr = other.m_pRoot;
            do {
                push_back(curr->getData(), curr->getRef());
                curr = curr->getNext();
            } while (curr != other.m_pRoot);
        }
        return *this;
    }

    CircularLinkedList &operator=(CircularLinkedList &&other) {
        if (this != &other) {
            clear();
            unique_lock<shared_mutex> lock(other.m_mtx);
            m_pRoot = std::exchange(other.m_pRoot, nullptr);
            m_tail  = std::exchange(other.m_tail,  nullptr);
            m_size  = std::exchange(other.m_size,  0);
        }
        return *this;
    }

    virtual ~CircularLinkedList() { clear(); }

    void clear() {
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) return;
        m_tail->setNext(nullptr);   // romper el circulo para poder recorrer
        Node *curr = m_pRoot;
        while (curr) {
            Node *next = curr->getNext();
            delete curr;
            curr = next;
        }
        m_pRoot = nullptr;
        m_tail  = nullptr;
        m_size  = 0;
    }

    void push_front(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(m_mtx);
        Node *newNode = new Node(value, ref);
        if (!m_pRoot) {
            newNode->setNext(newNode);
            m_pRoot = newNode;
            m_tail  = newNode;
        } else {
            newNode->setNext(m_pRoot);
            m_tail->setNext(newNode);   // reparar cierre circular
            m_pRoot = newNode;
        }
        m_size++;
    }

    void push_back(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(m_mtx);
        Node *newNode = new Node(value, ref);
        if (!m_pRoot) {
            newNode->setNext(newNode);
            m_pRoot = newNode;
            m_tail  = newNode;
        } else {
            newNode->setNext(m_pRoot);  // nuevo tail cierra el circulo
            m_tail->setNext(newNode);
            m_tail = newNode;
        }
        m_size++;
    }

    void insert(const value_type &value, Ref ref) override {
        unique_lock<shared_mutex> lock(m_mtx);
        internal_insert(value, ref);
    }

    std::tuple<value_type, Ref> pop_front() override {
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) throw runtime_error("La lista esta vacia");
        Node *temp   = m_pRoot;
        auto  result = std::make_tuple(temp->getData(), temp->getRef());
        if (m_size == 1) {
            m_pRoot = nullptr;
            m_tail  = nullptr;
        } else {
            m_pRoot = temp->getNext();
            m_tail->setNext(m_pRoot);   // reparar cierre circular
        }
        delete temp;
        m_size--;
        return result;
    }

    std::tuple<value_type, Ref> pop_back() override {
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) throw runtime_error("La lista esta vacia");
        auto result = std::make_tuple(m_tail->getData(), m_tail->getRef());
        if (m_size == 1) {
            delete m_tail;
            m_pRoot = nullptr;
            m_tail  = nullptr;
        } else {
            Node *act = m_pRoot;
            while (act->getNext() != m_tail) act = act->getNext();
            delete m_tail;
            m_tail = act;
            m_tail->setNext(m_pRoot);
        }
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

    forward_iterator begin() { return forward_iterator(this, m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr, true); }

    // ForEach: lock + delega al ForEach libre de util.h
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...args) {
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_size == 0) return;
        ::ForEach(begin(), end(), func, std::forward<Args>(args)...);
    }

    // circularForEach: recorre N vueltas completas sin restriccion del iterador
    template <typename Func, typename... Args>
    void circularForEach(size_t vueltas, Func func, Args &&...args) {
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot || vueltas == 0) return;
        Node  *act   = m_pRoot;
        size_t pasos = m_size * vueltas;
        for (size_t i = 0; i < pasos; ++i) {
            func(act->getDataRef(), std::forward<Args>(args)...);
            act = act->getNext();
        }
    }

    friend ostream &operator<<(ostream &os, const CircularLinkedList &list) {
        shared_lock<shared_mutex> lock(list.m_mtx);
        os << "[";
        if (list.m_pRoot) {
            Node *act = list.m_pRoot;
            do {
                os << "(" << act->getData() << "," << act->getRef() << ")";
                act = act->getNext();
                if (act != list.m_pRoot) os << ",";
            } while (act != list.m_pRoot);
        }
        os << "]";
        if (list.m_pRoot)
            os << " ->root(" << list.m_pRoot->getData() << ")";
        return os;
    }

    friend istream &operator>>(istream &is, CircularLinkedList &list) {
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

#endif // __CIRCULARLINKEDLIST_H__