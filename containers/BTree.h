#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <mutex>          
#include <shared_mutex>   
#include <utility>        
#include "BTreePage.h"

#define DEFAULT_BTREE_ORDER 3

using namespace std;

template <typename Trait>
class BTree {
public:
    using BTPage     = CBTreePage<Trait>;
    using keyType    = typename BTPage::keyType;
    using ObjIDType  = typename BTPage::ObjIDType;
    using ObjectInfo = typename BTPage::ObjectInfo;

public:
    BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
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
        return true;
    }

    ObjIDType Search(const keyType& key) {
        shared_lock<shared_mutex> lock(m_mtx);
        ObjIDType ObjID = 0;
        m_Root.Search(key, ObjID);
        return ObjID;
    }

    size_t size()     const { shared_lock<shared_mutex> lock(m_mtx); return m_NumKeys; }
    size_t height()   const { shared_lock<shared_mutex> lock(m_mtx); return m_Height;  }
    size_t GetOrder() const { shared_lock<shared_mutex> lock(m_mtx); return m_Order;   }

    template <typename Func, typename... Args>
    void ForEach(Func func, Args&&... args) {
        shared_lock<shared_mutex> lock(m_mtx);
        m_Root.ForEach(func, 0, forward<Args>(args)...);
    }

    template <typename Func, typename... Args>
    ObjectInfo* FirstThat(Func func, Args&&... args) {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_Root.FirstThat(func, 0, forward<Args>(args)...);
    }

    void Print(ostream& os) {
        this->ForEach([&os](const ObjectInfo& info, size_t level) {
            for (size_t i = 0; i < level; ++i) os << "\t";
            os << info.key << " -> " << info.ObjID << "\n";
        });
    }

protected:
    BTPage m_Root;
    size_t m_Height;
    size_t m_Order;
    size_t m_NumKeys;
    bool   m_Unique;
    mutable shared_mutex m_mtx; 
};

#endif