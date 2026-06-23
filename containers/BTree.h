#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include "BTreePage.h"
#include "traits.h"
#include "../types.h"
using namespace std;

constexpr auto DEFAULT_BTREE_ORDER = 3;

// Traits para BTree — siguen el mismo patron BaseTrait<_Node, _Comp>
template<typename T>
struct AscendingBTreeTrait  : public BaseTrait<tagObjectInfo<T>, less<T>> {};

template<typename T>
struct DescendingBTreeTrait : public BaseTrait<tagObjectInfo<T>, greater<T>> {};

template<typename Trait>
class BTree {
public:
    using value_type = typename Trait::value_type;
    using ObjIDType  = Ref;                                // siempre Ref = long
    using BTNode     = CBTreePage<value_type, ObjIDType>;
    using ObjectInfo = tagObjectInfo<value_type, ObjIDType>;

    BTree(int order = DEFAULT_BTREE_ORDER, bool unique = true);
    ~BTree() {}

    bool     Insert(const value_type key, const ObjIDType ObjID);
    bool     Remove(const value_type key, const ObjIDType ObjID);
    ObjIDType Search(const value_type key);

    long size()     { return m_NumKeys; }
    long height()   { return m_Height;  }
    long GetOrder() { return m_Order;   }

    void Print(ostream& os) { m_Root.Print(os); }

    // ForEach variadic — lambda/funcion recibe (ObjectInfo&, int level, args...)
    template<typename Func, typename... Args>
    void ForEach(Func func, Args&&... args) {
        m_Root.ForEach(func, 0, forward<Args>(args)...);
    }

    // FirstThat variadic — retorna el primer ObjectInfo que cumple la condicion
    template<typename Func, typename... Args>
    ObjectInfo* FirstThat(Func func, Args&&... args) {
        return m_Root.FirstThat(func, 0, forward<Args>(args)...);
    }

protected:
    BTNode m_Root;
    int    m_Height, m_Order;
    long   m_NumKeys;
    bool   m_Unique;
};

template<typename Trait>
BTree<Trait>::BTree(int order, bool unique)
    : m_Root(2 * order + 1, unique),
      m_Height(1),
      m_Order(order),
      m_NumKeys(0),
      m_Unique(unique) {
    m_Root.SetMaxKeysForChilds(order);
}

template<typename Trait>
bool BTree<Trait>::Insert(const value_type key, const ObjIDType ObjID) {
    auto error = m_Root.Insert(key, ObjID);
    if (error == bt_duplicate) return false;
    m_NumKeys++;
    if (error == bt_overflow) { m_Root.SplitRoot(); m_Height++; }
    return true;
}

template<typename Trait>
bool BTree<Trait>::Remove(const value_type key, const ObjIDType ObjID) {
    auto error = m_Root.Remove(key, ObjID);
    if (error == bt_duplicate || error == bt_nofound) return false;
    m_NumKeys--;
    if (error == bt_rootmerged) m_Height--;
    return true;
}

template<typename Trait>
typename BTree<Trait>::ObjIDType BTree<Trait>::Search(const value_type key) {
    ObjIDType ObjID = -1;
    m_Root.Search(key, ObjID);
    return ObjID;
}

#endif // BTREE_H