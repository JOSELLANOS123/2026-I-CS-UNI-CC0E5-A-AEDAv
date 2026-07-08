#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <memory>
#include <vector>
#include <mutex>          
#include <shared_mutex>   
#include <utility>        
#include "BTreePage.h"
#include "../types.h"

#define DEFAULT_BTREE_ORDER 3

using namespace std;


template <typename Trait>
class BTree {
public:
    using BTPage     = CBTreePage<Trait>;
    using keyType    = typename BTPage::keyType;
    using ObjIDType  = typename BTPage::ObjIDType;
    using ObjectInfo = typename BTPage::ObjectInfo;

    struct SnapEntry { ObjectInfo* info; T1 level; };

    
    class forward_iterator {
        shared_ptr<vector<SnapEntry>> m_snap;
        T1 m_i;
    public:
        forward_iterator() : m_i(0) {}
        forward_iterator(shared_ptr<vector<SnapEntry>> snap, T1 i) : m_snap(snap), m_i(i) {}
        ObjectInfo& operator*()  const { return *(*m_snap)[m_i].info; }
        ObjectInfo* operator->() const { return (*m_snap)[m_i].info; }
        T1 level() const { return (*m_snap)[m_i].level; }
        forward_iterator& operator++() { ++m_i; return *this; }
        bool operator!=(const forward_iterator& o) const { return m_i != o.m_i; }
        bool operator==(const forward_iterator& o) const { return m_i == o.m_i; }
    };

    
    class backward_iterator {
        shared_ptr<vector<SnapEntry>> m_snap;
        T1 m_i;
    public:
        backward_iterator() : m_i(0) {}
        backward_iterator(shared_ptr<vector<SnapEntry>> snap, T1 i) : m_snap(snap), m_i(i) {}
        ObjectInfo& operator*()  const { return *(*m_snap)[m_snap->size() - 1 - m_i].info; }
        ObjectInfo* operator->() const { return (*m_snap)[m_snap->size() - 1 - m_i].info; }
        T1 level() const { return (*m_snap)[m_snap->size() - 1 - m_i].level; }
        backward_iterator& operator++() { ++m_i; return *this; }
        bool operator!=(const backward_iterator& o) const { return m_i != o.m_i; }
        bool operator==(const backward_iterator& o) const { return m_i == o.m_i; }
    };

public:
    
    BTree(T1 order = DEFAULT_BTREE_ORDER, bool unique = true)
        : m_Root(2 * order + 1, unique), m_Height(1), m_Order(order), m_NumKeys(0), m_Unique(unique) {
    }

    virtual ~BTree() {}

    
    bool Insert(const keyType& key, const ObjIDType& ObjID) {
        unique_lock<shared_mutex> lock(m_mtx); 
        bt_ErrorCode error = m_Root.Insert(key, ObjID);
        if (error == bt_duplicate) return false;

        m_NumKeys++;
        if (error == bt_overflow) {
            m_Root.SplitRoot();
            m_Height++;
        }
        return true;
    }

   
    bool Remove(const keyType& key, const ObjIDType& ObjID) {
        unique_lock<shared_mutex> lock(m_mtx);
        bt_ErrorCode error = m_Root.Remove(key, ObjID);
        if (error == bt_duplicate || error == bt_nofound) return false;
        m_NumKeys--;
        if (error == bt_underflow && m_Root.MergeRoot() == bt_rootmerged) {
            m_Height--;
        }
        return true;
    }

    
    ObjIDType Search(const keyType& key) {
        shared_lock<shared_mutex> lock(m_mtx);
        ObjIDType ObjID = 0;
        m_Root.Search(key, ObjID);
        return ObjID;
    }

    T1 size()     const { shared_lock<shared_mutex> lock(m_mtx); return m_NumKeys; }
    T1 height()   const { shared_lock<shared_mutex> lock(m_mtx); return m_Height;  }
    T1 GetOrder() const { shared_lock<shared_mutex> lock(m_mtx); return m_Order;   }

    
    template <typename Func, typename... Args>
    void ForEach(Func func, Args&&... args) {
        for (auto it = begin(); it != end(); ++it)
            func(*it, it.level(), forward<Args>(args)...);
    }

    
    template <typename Func, typename... Args>
    ObjectInfo* FirstThat(Func func, Args&&... args) {
        for (auto it = begin(); it != end(); ++it)
            if (func(*it, it.level(), forward<Args>(args)...)) return &(*it);
        return nullptr;
    }

    
    void Print(ostream& os) {
        this->ForEach([&os](const ObjectInfo& info, T1 level) {
            for (T1 i = 0; i < level; ++i) os << "\t";
            os << info.key << " -> " << info.ObjID << "\n";
        });
    }

    // begin/end: iterador ascendente (forward). Cada llamada arma su propio snapshot (foto del arbol en ese instante).
    forward_iterator begin() {
        shared_lock<shared_mutex> lock(m_mtx);
        return forward_iterator(Snapshot(), 0);
    }
    forward_iterator end() {
        shared_lock<shared_mutex> lock(m_mtx);
        auto snap = Snapshot();
        return forward_iterator(snap, snap->size());
    }

    // rbegin/rend: iterador descendente (backward), mismo snapshot al reves.
    backward_iterator rbegin() {
        shared_lock<shared_mutex> lock(m_mtx);
        return backward_iterator(Snapshot(), 0);
    }
    backward_iterator rend() {
        shared_lock<shared_mutex> lock(m_mtx);
        auto snap = Snapshot();
        return backward_iterator(snap, snap->size());
    }

    
    friend ostream& operator<<(ostream& os, BTree& bt) {
        os << "[";
        bool first = true;
        for (auto it = bt.begin(); it != bt.end(); ++it) {
            if (!first) os << ", ";
            os << "(" << it->key << ":" << it->ObjID << ")";
            first = false;
        }
        return os << "]";
    }

    // Lee el formato de operator<< y reinserta cada (clave:valor) en el arbol.
    friend istream& operator>>(istream& is, BTree& bt) {
        Character ch;
        if (!(is >> ch) || ch != '[') { is.setstate(ios_base::failbit); return is; }
        if ((is >> ws).peek() == ']') { is >> ch; return is; } // arbol vacio: "[]"

        keyType k; ObjIDType v; Character open, colon, close;
        while (is >> open >> k >> colon >> v >> close) {
            if (open != '(' || colon != ':' || close != ')') {
                is.setstate(ios_base::failbit);
                break;
            }
            bt.Insert(k, v);
            is >> ch;
            if (ch == ']') break;
            if (ch != ',') { is.setstate(ios_base::failbit); break; }
        }
        return is;
    }

protected:
    // Arma el snapshot (foto en orden de todas las claves) reutilizando el ForEach que ya existe en CBTreePage 
    // Este snapshot es lo unico que alimenta a begin/end/rbegin/rend.
    shared_ptr<vector<SnapEntry>> Snapshot() {
        auto snap = make_shared<vector<SnapEntry>>();
        m_Root.ForEach([&snap](ObjectInfo& info, T1 level) {
            snap->push_back({&info, level});
        }, 0);
        return snap;
    }

    BTPage m_Root;
    T1 m_Height;   
    T1 m_Order;    
    T1 m_NumKeys;  
    bool   m_Unique;
    mutable shared_mutex m_mtx;  
};

#endif