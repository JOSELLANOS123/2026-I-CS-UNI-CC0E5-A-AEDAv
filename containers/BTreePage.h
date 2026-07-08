#ifndef CBTreePage_H
#define CBTreePage_H

#include <vector>
#include <iostream>
#include <cassert>
#include <utility>
#include "../types.h"


enum bt_ErrorCode { bt_ok, bt_overflow, bt_underflow, bt_duplicate, bt_nofound, bt_rootmerged };


template<typename K, typename V = Ref>
struct tagObjectInfo {
    using value_type = K;
    using id_type    = V;
    K key;
    V ObjID;
    Ref UseCounter = 0;
    tagObjectInfo(const K& _key, V _ObjID) : key(_key), ObjID(_ObjID), UseCounter(0) {}
    tagObjectInfo() {}
    operator K() const { return key; }
};

// (nodo) del arbol B
template<typename Trait>
class CBTreePage {
    template<typename T> friend class BTree;

public:
    using ObjectInfo = typename Trait::Node;
    using keyType    = typename ObjectInfo::value_type;
    using ObjIDType  = typename ObjectInfo::id_type;
    using BTPage     = CBTreePage<Trait>;
    using Comp       = typename Trait::Comp;

public:
    CBTreePage(T1 maxKeys, bool unique = true, Comp comp = Comp());
    virtual ~CBTreePage();

    bt_ErrorCode Insert(const keyType& key, const ObjIDType& ObjID); // inserta key->ObjID
    bt_ErrorCode Remove(const keyType& key, const ObjIDType& ObjID); // elimina key
    bool         Search(const keyType& key, ObjIDType& ObjID);       // busca key

    // Recorrido in-order; unico mecanismo que usan ForEach/FirstThat/iterador/<<
    template <typename Func, typename... Args>
    void ForEach(Func func, T1 level, Args&&... args) {
        T1 i = 0;
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

protected:
    T1 m_MaxKeys;                       
    T1 m_KeyCount;                      
    bool m_Unique;                      
    std::vector<ObjectInfo> m_Keys;
    std::vector<BTPage*> m_SubPages;
    Comp m_cmp;

    T1 binary_search(const keyType& key) const;
    void insert_at(T1 pos, const ObjectInfo& info); 
    void remove_at(T1 pos);                         

    bool Underflow() const { return m_KeyCount < m_MaxKeys / 2; }
    bool Overflow() const { return m_KeyCount > m_MaxKeys; }

    void SplitChild(T1 pos);   
    bool SplitRoot();          
    bt_ErrorCode MergeRoot();  
    void Destroy();           
    void clear();               
    void MovePage(BTPage* source, std::vector<ObjectInfo>& tmpK, std::vector<BTPage*>& tmpP); 
    ObjectInfo& GetFirstObjectInfo(); 

    T1 MinKeys() const { return m_MaxKeys / 2; }

    void Restore(T1 childIdx);              // decide pedir prestado o fusionar
    void RedistributeFromLeft(T1 childIdx);  
    void RedistributeFromRight(T1 childIdx);
    void MergeChildren(T1 leftIdx);          
};

template<typename T>
CBTreePage<T>::CBTreePage(T1 maxKeys, bool unique, Comp comp)
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
    for (T1 i = 0; i <= m_KeyCount; i++) {
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
T1 CBTreePage<T>::binary_search(const keyType& key) const {
    T1 first = 0;
    T1 last = m_KeyCount;
    while (first < last) {
        T1 mid = first + (last - first) / 2;
        if (!m_cmp(key, m_Keys[mid].key) && !m_cmp(m_Keys[mid].key, key)) return mid;
        if (m_cmp(m_Keys[mid].key, key)) first = mid + 1;
        else last = mid;
    }
    return first;
}

template<typename T>
void CBTreePage<T>::insert_at(T1 pos, const ObjectInfo& info) {
    for (T1 i = m_KeyCount; i > pos; i--) {
        m_Keys[i] = m_Keys[i - 1];
        m_SubPages[i + 1] = m_SubPages[i];
    }
    m_Keys[pos] = info;
    m_KeyCount++;
}

template<typename T>
void CBTreePage<T>::remove_at(T1 pos) {
    for (T1 i = pos; i < m_KeyCount - 1; i++) {
        m_Keys[i] = m_Keys[i + 1];
        m_SubPages[i + 1] = m_SubPages[i + 2];
    }
    m_KeyCount--;
}

template<typename T>
bt_ErrorCode CBTreePage<T>::Insert(const keyType& key, const ObjIDType& ObjID) {
    T1 pos = binary_search(key);
    if (pos < m_KeyCount && !m_cmp(key, m_Keys[pos].key) && !m_cmp(m_Keys[pos].key, key) && m_Unique) {
        return bt_duplicate;
    }
    if (!m_SubPages[pos]) { // hoja: se inserta aqui
        insert_at(pos, ObjectInfo(key, ObjID));
        return Overflow() ? bt_overflow : bt_ok;
    } else { // baja al hijo; si se desborda, se divide
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
    T1 mid = m_KeyCount / 2;
    ObjectInfo midKey = m_Keys[mid];

    for (T1 i = 0; i < mid; i++) {
        newChild1->m_Keys[i] = m_Keys[i];
        newChild1->m_SubPages[i] = m_SubPages[i];
        newChild1->m_KeyCount++;
    }
    newChild1->m_SubPages[mid] = m_SubPages[mid];

    for (T1 i = mid + 1; i < m_KeyCount; i++) {
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
    T1 pos = binary_search(key);
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
    T1 pos = binary_search(key);
    if (pos < m_KeyCount && !m_cmp(key, m_Keys[pos].key) && !m_cmp(m_Keys[pos].key, key)) {
        if (!m_SubPages[pos]) { // hoja: se borra directo
            remove_at(pos);
            return Underflow() ? bt_underflow : bt_ok;
        }
        // interna: se reemplaza por el sucesor y se borra este de su lugar
        ObjectInfo& successor = m_SubPages[pos + 1]->GetFirstObjectInfo();
        std::swap(m_Keys[pos], successor);
        bt_ErrorCode err = m_SubPages[pos + 1]->Remove(key, ObjID);
        if (err == bt_underflow) {
            Restore(pos + 1);
            return Underflow() ? bt_underflow : bt_ok;
        }
        return err;
    }
    if (m_SubPages[pos]) {
        bt_ErrorCode err = m_SubPages[pos]->Remove(key, ObjID);
        if (err == bt_underflow) {
            Restore(pos);
            return Underflow() ? bt_underflow : bt_ok;
        }
        return err;
    }
    return bt_nofound;
}

template<typename T>
void CBTreePage<T>::SplitChild(T1 pos) {
    BTPage* child = m_SubPages[pos];
    BTPage* newChild = new BTPage(m_MaxKeys, m_Unique, m_cmp);
    T1 mid = child->m_KeyCount / 2;
    ObjectInfo midKey = child->m_Keys[mid];

    for (T1 i = mid + 1; i < child->m_KeyCount; i++) {
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
    if (m_KeyCount != 0 || !m_SubPages[0]) return bt_ok; // no aplica: no esta vacia o no tiene hijo

    BTPage* onlyChild = m_SubPages[0];
    T1 n = onlyChild->m_KeyCount;
    for (T1 i = 0; i < n; i++) {
        m_Keys[i]     = onlyChild->m_Keys[i];
        m_SubPages[i] = onlyChild->m_SubPages[i];
        onlyChild->m_SubPages[i] = nullptr; // evita doble delete
    }
    m_SubPages[n] = onlyChild->m_SubPages[n];
    onlyChild->m_SubPages[n] = nullptr;
    m_KeyCount = n;

    delete onlyChild;
    return bt_rootmerged;
}

template<typename T>
void CBTreePage<T>::Restore(T1 childIdx) {
    if (childIdx > 0 && m_SubPages[childIdx - 1]->m_KeyCount > m_SubPages[childIdx - 1]->MinKeys()) {
        RedistributeFromLeft(childIdx);
        return;
    }
    if (childIdx < m_KeyCount && m_SubPages[childIdx + 1]->m_KeyCount > m_SubPages[childIdx + 1]->MinKeys()) {
        RedistributeFromRight(childIdx);
        return;
    }
    MergeChildren(childIdx > 0 ? childIdx - 1 : childIdx);
}

template<typename T>
void CBTreePage<T>::RedistributeFromLeft(T1 childIdx) {
    BTPage* child = m_SubPages[childIdx];
    BTPage* left  = m_SubPages[childIdx - 1];

    T1 n = child->m_KeyCount;
    for (T1 i = n; i > 0; i--) child->m_Keys[i] = child->m_Keys[i - 1];
    for (T1 i = n + 1; i > 0; i--) child->m_SubPages[i] = child->m_SubPages[i - 1];
    child->m_KeyCount++;

    child->m_Keys[0]     = m_Keys[childIdx - 1];
    child->m_SubPages[0] = left->m_SubPages[left->m_KeyCount];

    m_Keys[childIdx - 1] = left->m_Keys[left->m_KeyCount - 1];
    left->m_KeyCount--;
}

template<typename T>
void CBTreePage<T>::RedistributeFromRight(T1 childIdx) {
    BTPage* child = m_SubPages[childIdx];
    BTPage* right = m_SubPages[childIdx + 1];

    child->m_Keys[child->m_KeyCount]         = m_Keys[childIdx];
    child->m_SubPages[child->m_KeyCount + 1] = right->m_SubPages[0];
    child->m_KeyCount++;

    m_Keys[childIdx] = right->m_Keys[0];

    T1 n = right->m_KeyCount;
    for (T1 i = 0; i < n - 1; i++) right->m_Keys[i] = right->m_Keys[i + 1];
    for (T1 i = 0; i < n; i++)     right->m_SubPages[i] = right->m_SubPages[i + 1];
    right->m_KeyCount--;
}

template<typename T>
void CBTreePage<T>::MergeChildren(T1 leftIdx) {
    BTPage* left  = m_SubPages[leftIdx];
    BTPage* right = m_SubPages[leftIdx + 1];

    left->m_Keys[left->m_KeyCount] = m_Keys[leftIdx];
    left->m_KeyCount++;

    for (T1 i = 0; i < right->m_KeyCount; i++) {
        left->m_Keys[left->m_KeyCount]     = right->m_Keys[i];
        left->m_SubPages[left->m_KeyCount] = right->m_SubPages[i];
        right->m_SubPages[i] = nullptr; // evita doble delete
        left->m_KeyCount++;
    }
    left->m_SubPages[left->m_KeyCount] = right->m_SubPages[right->m_KeyCount];
    right->m_SubPages[right->m_KeyCount] = nullptr;

    delete right;
    remove_at(leftIdx); // quita el separador y desplaza los punteros restantes
}

template<typename T>
typename CBTreePage<T>::ObjectInfo& CBTreePage<T>::GetFirstObjectInfo() {
    if (m_SubPages[0]) return m_SubPages[0]->GetFirstObjectInfo();
    return m_Keys[0];
}

template<typename T>
void CBTreePage<T>::MovePage(BTPage* source, std::vector<ObjectInfo>& tmpK, std::vector<BTPage*>& tmpP) {
    for (T1 i = 0; i < source->m_KeyCount; i++) {
        tmpK.push_back(source->m_Keys[i]);
        tmpP.push_back(source->m_SubPages[i]);
    }
    tmpP.push_back(source->m_SubPages[source->m_KeyCount]);
    source->clear();
}

#endif