#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <utility>
#include "BTreePage.h"
#include "traits.h"
#include "../types.h"
using namespace std;

constexpr auto DEFAULT_BTREE_ORDER = 3;

template<typename Trait>
class BTree {
    using keyType    = typename Trait::keyType;
    using ObjIDType  = typename Trait::ObjIDType;
    using BTNode     = CBTreePage<Trait>;

public:
    using value_type = typename Trait::value_type;   
    using ObjectInfo = tagObjectInfo<keyType, ObjIDType>;

    BTree(int order = DEFAULT_BTREE_ORDER, bool unique = true);
    ~BTree() {}

    bool      Insert(const keyType key, const ObjIDType ObjID);
    bool      Remove(const keyType key, const ObjIDType ObjID);
    ObjIDType Search(const keyType key);

    long size()     { return m_NumKeys; }
    long height()   { return m_Height;  }
    long GetOrder() { return m_Order;   }

    void Print(ostream& os) { m_Root.Print(os); }

    // ForEach variadic — delega a CBTreePage, nivel comienza en 0
    template<typename Func, typename... Args>
    void ForEach(Func func, Args&&... args) {
        m_Root.ForEach(func, 0, forward<Args>(args)...);
    }

    // FirstThat variadic — retorna el primer ObjectInfo que cumple func
    template<typename Func, typename... Args>
    ObjectInfo* FirstThat(Func func, Args&&... args) {
        return m_Root.FirstThat(func, 0, forward<Args>(args)...);
    }

protected:
    bool   m_Unique;
    int    m_Order;
    BTNode m_Root;
    int    m_Height;
    long   m_NumKeys;
};

template<typename Trait>
BTree<Trait>::BTree(int order, bool unique)
    : m_Unique(unique), m_Order(order),
      m_Root(2 * order + 1, unique), m_Height(1), m_NumKeys(0) {
    m_Root.SetMaxKeysForChilds(order);
}

template<typename Trait>
bool BTree<Trait>::Insert(const keyType key, const ObjIDType ObjID) {
    auto error = m_Root.Insert(key, ObjID);
    if (error == bt_duplicate) return false;
    m_NumKeys++;
    if (error == bt_overflow) { m_Root.SplitRoot(); m_Height++; }
    return true;
}

template<typename Trait>
bool BTree<Trait>::Remove(const keyType key, const ObjIDType ObjID) {
    auto error = m_Root.Remove(key, ObjID);
    if (error == bt_duplicate || error == bt_nofound) return false;
    m_NumKeys--;
    if (error == bt_rootmerged) m_Height--;
    return true;
}

template<typename Trait>
typename BTree<Trait>::ObjIDType BTree<Trait>::Search(const keyType key) {
    ObjIDType ObjID = -1;
    m_Root.Search(key, ObjID);
    return ObjID;
}

#endif // BTREE_H