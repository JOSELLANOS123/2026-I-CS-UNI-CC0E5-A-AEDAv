#ifndef __HEAP_H__
#define __HEAP_H__

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <shared_mutex>
#include <mutex>
#include <tuple>
#include <utility>
#include "vector.h"
#include "traits.h"
#include "../types.h"
using namespace std;

// Traits para Heap — usan VectorNode<T> que ahora tiene value_type (adaptacion)
template <typename T>
struct MinHeapTrait : public BaseTrait<VectorNode<T>, less<T>> {};

template <typename T>
struct MaxHeapTrait : public BaseTrait<VectorNode<T>, greater<T>> {};

template<typename Trait>
class Heap {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;        // VectorNode<value_type>
    using Comp       = typename Trait::Comp;
    using MySelf     = Heap<Trait>;

private:
    Vector<value_type> m_vec;   // Vector adaptado: almacena VectorNode<value_type>
    Comp               m_comp;
    mutable shared_mutex m_mtx;

    
    size_t parent(size_t i) const { return (i - 1) / 2; }
    size_t left  (size_t i) const { return 2 * i + 1;   }
    size_t right (size_t i) const { return 2 * i + 2;   }

    // Internos: se llaman con m_mtx ya adquirido
    void heapifyUp(size_t i) {
        while (i > 0 && m_comp(m_vec[i].getData(), m_vec[parent(i)].getData())) {
            m_vec.swap_at(i, parent(i));
            i = parent(i);
        }
    }

    void heapifyDown(size_t i) {
        auto n = m_vec.size();
        while (true) {
            auto best = i;
            auto l = left(i), r = right(i);
            if (l < n && m_comp(m_vec[l].getData(), m_vec[best].getData())) best = l;
            if (r < n && m_comp(m_vec[r].getData(), m_vec[best].getData())) best = r;
            if (best == i) break;
            m_vec.swap_at(i, best);
            i = best;
        }
    }

public:
    Heap() : m_vec(), m_comp() {}

    // Copy constructor
    Heap(const Heap& other) : m_comp(other.m_comp) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        for (size_t i = 0; i < other.m_vec.size(); ++i)
            m_vec.push_back(other.m_vec[i].getData(), other.m_vec[i].getRef());
    }

    // Move constructor
    Heap(Heap&& other) : m_comp(other.m_comp) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        for (size_t i = 0; i < other.m_vec.size(); ++i)
            m_vec.push_back(other.m_vec[i].getData(), other.m_vec[i].getRef());
        // vaciar el otro
        while (!other.m_vec.empty()) other.m_vec.pop_back();
    }

    virtual ~Heap() {}

    void insert(const value_type& data, Ref ref) {
        unique_lock<shared_mutex> lock(m_mtx);
        m_vec.push_back(data, ref);
        heapifyUp(m_vec.size() - 1);
    }

    // extract: retorna el elemento de mayor prioridad y lo elimina
    tuple<value_type, Ref> extract() {
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_vec.empty()) throw runtime_error("heap vacio");
        auto rv = make_tuple(m_vec[0].getData(), m_vec[0].getRef());
        m_vec.swap_at(0, m_vec.size() - 1);
        m_vec.pop_back();
        if (!m_vec.empty()) heapifyDown(0);
        return rv;
    }

    
    tuple<value_type, Ref> peek() const {
        shared_lock<shared_mutex> lock(m_mtx);
        if (m_vec.empty()) throw runtime_error("heap vacio");
        return make_tuple(m_vec[0].getData(), m_vec[0].getRef());
    }

    bool   isEmpty() const { return m_vec.empty(); }
    size_t size()    const { return m_vec.size(); }

    // toString 
    string toString() const {
        shared_lock<shared_mutex> lock(m_mtx);
        ostringstream os; os << "[";
        for (size_t i = 0; i < m_vec.size(); ++i) {
            if (i) os << ",";
            os << "(" << m_vec[i].getData() << "," << m_vec[i].getRef() << ")";
        }
        return os.str() + "]";
    }

    // operator<< 
    friend ostream& operator<<(ostream& os, const Heap& h) {
        return os << h.toString();
    }

    // operator>> 
    friend istream& operator>>(istream& is, Heap& h) {
        char ch;
        if (!(is >> ch) || ch != '[') { is.clear(ios_base::failbit); return is; }
        value_type d; Ref r; char cm, cp;
        while (is >> ch && ch != ']')
            if (ch == '(' && is >> d >> cm >> r >> cp && cm == ',' && cp == ')')
                h.insert(d, r);
        return is;
    }
};

void DemoHeap();

#endif // __HEAP_H__