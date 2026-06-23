#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include <iostream>
#include <sstream>
#include <shared_mutex>
#include <utility>
#include <memory>
#include <vector>
#include "avl.h"
#include "linkedlist.h"  // tercera estructura para colisiones
using namespace std;

// HashTable: simulacion de hash usando un AVL como estructura principal
// Si hay colisiones (misma clave), se resuelven con una LinkedList (tercera estructura)
template<typename Trait>
class HashTable {
public:
    using value_type = typename Trait::value_type;
    using Entry      = pair<value_type, Ref>;   // par (clave, valor) para iteracion

private:
    AVL<Trait>     m_avl;    // estructura principal — donde se guarda la informacion
    mutable shared_mutex m_mtx;

    // Tercera estructura: LinkedList para colisiones
    // Si una clave ya existe en el AVL, la colision se registra aqui
    LinkedList<AscendingTrait<LLNode<value_type>>> m_collisions;

    // Iterador snapshot sobre los entries del AVL — soporta structured bindings via pair
    struct HTIter {
        shared_ptr<vector<Entry>> m_snap;
        size_t m_i;
        const Entry& operator*()  const { return (*m_snap)[m_i]; }
        HTIter& operator++()            { ++m_i; return *this; }
        bool operator!=(const HTIter& o) const { return m_i != o.m_i; }
        bool operator==(const HTIter& o) const { return m_i == o.m_i; }
    };

    mutable shared_ptr<vector<Entry>> m_snap;

    vector<Entry> all_entries() const {
        vector<Entry> entries;
        auto view = m_avl.inorder();
        for (auto it = view.begin(); it != view.end(); ++it)
            entries.push_back({it.getNode()->m_data, it.getNode()->m_ref});
        return entries;
    }

public:
    HashTable() {}

    // Constructor copia
    HashTable(const HashTable& other) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_avl        = other.m_avl;
        m_collisions = other.m_collisions;
    }

    // Move constructor — exchange para el size, move en el AVL
    HashTable(HashTable&& other) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_avl        = move(other.m_avl);
        m_collisions = move(other.m_collisions);
    }

    HashTable& operator=(const HashTable& other) {
        if (this != &other) {
            shared_lock<shared_mutex> lock(other.m_mtx);
            m_avl        = other.m_avl;
            m_collisions = other.m_collisions;
        }
        return *this;
    }
    HashTable& operator=(HashTable&& other) {
        if (this != &other) {
            unique_lock<shared_mutex> lock(other.m_mtx);
            m_avl        = move(other.m_avl);
            m_collisions = move(other.m_collisions);
        }
        return *this;
    }

    // operator[] — delega hacia insert en el AVL interno
    // Si la clave ya existe: actualiza su valor (sobrescribe)
    // Si es nueva: inserta en el AVL
    Ref& operator[](const value_type& key) {
        return m_avl[key];   // AVL::operator[] busca o inserta y retorna Ref&
    }

    size_t size() const { return m_avl.size(); }

    // begin/end para: for (const auto& [key, value] : tabla)
    HTIter begin() const {
        shared_lock<shared_mutex> lock(m_mtx);
        m_snap = make_shared<vector<Entry>>(all_entries());
        return {m_snap, 0};
    }
    HTIter end() const {
        return {m_snap, m_snap ? m_snap->size() : 0};
    }

    // operator<< — muestra los pares (key:value) en orden del AVL
    friend ostream& operator<<(ostream& os, const HashTable& ht) {
        os << "{";
        bool first = true;
        for (const auto& [k, v] : const_cast<HashTable&>(ht)) {
            if (!first) os << ",";
            os << "(" << k << ":" << v << ")";
            first = false;
        }
        return os << "}";
    }

    // operator>> — parsea {(k:v),...} e inserta via operator[]
    friend istream& operator>>(istream& is, HashTable& ht) {
        auto ch = char{}, colon = char{}, cp = char{};
        if (!(is >> ch) || ch != '{') { is.clear(ios_base::failbit); return is; }
        value_type k; Ref v;
        while (is >> ch && ch != '}')
            if (ch == '(' && is >> k >> colon >> v >> cp && colon == ':' && cp == ')')
                ht[k] = v;
        return is;
    }
};

void DemoHashTable();

#endif // __HASHTABLE_H__