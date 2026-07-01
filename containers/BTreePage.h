#ifndef CBTreePage_H
#define CBTreePage_H

#include <vector>
#include <iostream>
#include <cassert>
#include <utility>      

enum bt_ErrorCode { bt_ok, bt_overflow, bt_underflow, bt_duplicate, bt_nofound, bt_rootmerged };

template<typename K, typename V = long>
struct tagObjectInfo {
    using value_type = K;
    using id_type    = V;
    K key;
    V ObjID;
    long UseCounter = 0;
    tagObjectInfo(const K& _key, V _ObjID) : key(_key), ObjID(_ObjID), UseCounter(0) {}
    tagObjectInfo() {}
    operator K() const { return key; }
};

template<typename Trait>
class CBTreePage {
    template<typename T> friend class BTree;
    
public:
    // Extraemos la estructura contenedora del nodo desde el Trait
    using ObjectInfo = typename Trait::Node; 
    using keyType    = typename ObjectInfo::value_type;
    using ObjIDType  = typename ObjectInfo::id_type;
    using BTPage     = CBTreePage<Trait>;
    using Comp       = typename Trait::Comp;

public:
    CBTreePage(int maxKeys, bool unique = true, Comp comp = Comp());
    virtual ~CBTreePage();

    bt_ErrorCode Insert(const keyType& key, const ObjIDType& ObjID);
    bt_ErrorCode Remove(const keyType& key, const ObjIDType& ObjID);
    bool         Search(const keyType& key, ObjIDType& ObjID);

    template <typename Func, typename... Args>
    void ForEach(Func func, size_t level, Args&&... args) {
        size_t i = 0;
        for (; i < m_KeyCount; ++i) {
            if (m_SubPages[i]) {
                m_SubPages[i]->ForEach(func, level + 1, std::forward<Args>(args)...);
            }
            func(m_Keys[i], level, std::forward<Args>(args)...);
        }
        if (m_SubPages[i]) {
            m_SubPages[i]->ForEach(func, level + 1, std::forward<Args>(args)...);
        }
    }

    template <typename Func, typename... Args>
    ObjectInfo* FirstThat(Func func, size_t level, Args&&... args) {
        size_t i = 0;
        for (; i < m_KeyCount; ++i) {
            if (m_SubPages[i]) {
                ObjectInfo* res = m_SubPages[i]->FirstThat(func, level + 1, std::forward<Args>(args)...);
                if (res) return res;
            }
            if (func(m_Keys[i], level, std::forward<Args>(args)...)) {
                return &m_Keys[i];
            }
        }
        if (m_SubPages[i]) {
            return m_SubPages[i]->FirstThat(func, level + 1, std::forward<Args>(args)...);
        }
        return nullptr;
    }

protected:
    int m_MaxKeys;
    int m_KeyCount;
    bool m_Unique;
    std::vector<ObjectInfo> m_Keys;
    std::vector<BTPage*> m_SubPages;
    Comp m_cmp;

    int binary_search(const keyType& key) const;
    void insert_at(int pos, const ObjectInfo& info);
    void remove_at(int pos);
    bool Underflow() const { return m_KeyCount < m_MaxKeys / 2; }
    bool Overflow() const { return m_KeyCount > m_MaxKeys; }
    
    void SplitChild(int pos);
    bool SplitRoot();
    bt_ErrorCode MergeRoot();
    void Destroy();
    void clear();
    void MovePage(BTPage* source, std::vector<ObjectInfo>& tmpK, std::vector<BTPage*>& tmpP);
    ObjectInfo& GetFirstObjectInfo();
};

template<typename T>
CBTreePage<T>::CBTreePage(int maxKeys, bool unique, Comp comp)
    : m_MaxKeys(maxKeys), m_KeyCount(0), m_Unique(unique), m_cmp(comp) {
    m_Keys.resize(m_MaxKeys + 1);
    m_SubPages.resize(m_MaxKeys + 2, nullptr);
}

template<typename T>
CBTreePage<T>::~CBTreePage() {
    Destroy();
}

template<typename T>
void CBTreePage<T>::Destroy() {
    for (int i = 0; i <= m_KeyCount; i++) {
        if (m_SubPages[i] != nullptr) {
            delete m_SubPages[i];
            m_SubPages[i] = nullptr;
        }
    }
    clear();
}

template<typename T>
void CBTreePage<T>::clear() {
    m_KeyCount = 0;
}

template<typename T>
int CBTreePage<T>::binary_search(const keyType& key) const {
    int first = 0;
    int last = m_KeyCount;
    while (first < last) {
        int mid = first + (last - first) / 2;
        if (!m_cmp(key, m_Keys[mid].key) && !m_cmp(m_Keys[mid].key, key)) return mid;
        if (m_cmp(m_Keys[mid].key, key)) first = mid + 1;
        else last = mid;
    }
    return first;
}

template<typename T>
void CBTreePage<T>::insert_at(int pos, const ObjectInfo& info) {
    for (int i = m_KeyCount; i > pos; i--) {
        m_Keys[i] = m_Keys[i - 1];
        m_SubPages[i + 1] = m_SubPages[i];
    }
    m_Keys[pos] = info;
    m_KeyCount++;
}

template<typename T>
void CBTreePage<T>::remove_at(int pos) {
    for (int i = pos; i < m_KeyCount - 1; i++) {
        m_Keys[i] = m_Keys[i + 1];
        m_SubPages[i + 1] = m_SubPages[i + 2];
    }
    m_KeyCount--;
}

template<typename T>
bt_ErrorCode CBTreePage<T>::Insert(const keyType& key, const ObjIDType& ObjID) {
    int pos = binary_search(key);
    if (pos < m_KeyCount && !m_cmp(key, m_Keys[pos].key) && !m_cmp(m_Keys[pos].key, key) && m_Unique) {
        return bt_duplicate;
    }
    if (!m_SubPages[pos]) {
        insert_at(pos, ObjectInfo(key, ObjID));
        return Overflow() ? bt_overflow : bt_ok;
    } else {
        bt_ErrorCode err = m_SubPages[pos]->Insert(key, ObjID);
        if (err == bt_overflow) {
            SplitChild(pos);
            return Overflow() ? bt_overflow : bt_ok;
        }
        return err;
    }
}

template<typename T>
bool CBTreePage<T>::SplitRoot() {
    if (!Overflow()) return false;
    BTPage* newChild1 = new BTPage(m_MaxKeys, m_Unique, m_cmp);
    BTPage* newChild2 = new BTPage(m_MaxKeys, m_Unique, m_cmp);
    int mid = m_KeyCount / 2;
    ObjectInfo midKey = m_Keys[mid];

    for (int i = 0; i < mid; i++) {
        newChild1->m_Keys[i] = m_Keys[i];
        newChild1->m_SubPages[i] = m_SubPages[i];
        newChild1->m_KeyCount++;
    }
    newChild1->m_SubPages[mid] = m_SubPages[mid];

    for (int i = mid + 1; i < m_KeyCount; i++) {
        newChild2->m_Keys[i - (mid + 1)] = m_Keys[i];
        newChild2->m_SubPages[i - (mid + 1)] = m_SubPages[i];
        newChild2->m_KeyCount++;
    }
    newChild2->m_SubPages[m_KeyCount - (mid + 1)] = m_SubPages[m_KeyCount];

    clear();
    m_Keys[0] = midKey;
    m_SubPages[0] = newChild1;
    m_SubPages[1] = newChild2;
    m_KeyCount = 1;
    return true;
}

template<typename T>
bool CBTreePage<T>::Search(const keyType& key, ObjIDType& ObjID) {
    int pos = binary_search(key);
    if (pos < m_KeyCount && !m_cmp(key, m_Keys[pos].key) && !m_cmp(m_Keys[pos].key, key)) {
        ObjID = m_Keys[pos].ObjID;
        m_Keys[pos].UseCounter++;
        return true;
    }
    if (m_SubPages[pos]) return m_SubPages[pos]->Search(key, ObjID);
    return false;
}

template<typename T>
bt_ErrorCode CBTreePage<T>::Remove(const keyType& key, const ObjIDType& ObjID) {
    int pos = binary_search(key);
    if (pos < m_KeyCount && !m_cmp(key, m_Keys[pos].key) && !m_cmp(m_Keys[pos].key, key)) {
        if (!m_SubPages[pos]) {
            remove_at(pos);
            return Underflow() ? bt_underflow : bt_ok;
        }
        ObjectInfo& successor = m_SubPages[pos + 1]->GetFirstObjectInfo();
        std::swap(m_Keys[pos], successor);
        return m_SubPages[pos + 1]->Remove(key, ObjID);
    }
    if (m_SubPages[pos]) return m_SubPages[pos]->Remove(key, ObjID);
    return bt_nofound;
}

template<typename T>
void CBTreePage<T>::SplitChild(int pos) {
    BTPage* child = m_SubPages[pos];
    BTPage* newChild = new BTPage(m_MaxKeys, m_Unique, m_cmp);
    int mid = child->m_KeyCount / 2;
    ObjectInfo midKey = child->m_Keys[mid];

    for (int i = mid + 1; i < child->m_KeyCount; i++) {
        newChild->m_Keys[i - (mid + 1)] = child->m_Keys[i];
        newChild->m_SubPages[i - (mid + 1)] = child->m_SubPages[i];
        newChild->m_KeyCount++;
    }
    newChild->m_SubPages[child->m_KeyCount - (mid + 1)] = child->m_SubPages[child->m_KeyCount];
    child->m_KeyCount = mid;

    insert_at(pos, midKey);
    m_SubPages[pos + 1] = newChild;
}

template<typename T>
bt_ErrorCode CBTreePage<T>::MergeRoot() {
    return bt_rootmerged;
}

template<typename T>
typename CBTreePage<T>::ObjectInfo& CBTreePage<T>::GetFirstObjectInfo() {
    if (m_SubPages[0]) return m_SubPages[0]->GetFirstObjectInfo();
    return m_Keys[0];
}

template<typename T>
void CBTreePage<T>::MovePage(BTPage* source, std::vector<ObjectInfo>& tmpK, std::vector<BTPage*>& tmpP) {
    for (int i = 0; i < source->m_KeyCount; i++) {
        tmpK.push_back(source->m_Keys[i]);
        tmpP.push_back(source->m_SubPages[i]);
    }
    tmpP.push_back(source->m_SubPages[source->m_KeyCount]);
    source->clear();
}

#endif