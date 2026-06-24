#ifndef CBTreePage_H
#define CBTreePage_H

#include <vector>
#include <iostream>
#include <cassert>
#include <utility>
#include "../types.h"
using namespace std;

template <typename Trait> class BTree;

enum bt_ErrorCode { bt_ok, bt_overflow, bt_underflow, bt_duplicate, bt_nofound, bt_rootmerged };

// tagObjectInfo: mantiene los dos parametros originales (K, V)
// Agrega value_type para conectar con el sistema de Traits del proyecto
template<typename K, typename V = long>
struct tagObjectInfo {
    using value_type = K;         // consistencia con BaseTrait del proyecto
    K    key;
    V    ObjID;
    long UseCounter;
    tagObjectInfo(const K& _key, V _ObjID) : key(_key), ObjID(_ObjID), UseCounter(0) {}
    tagObjectInfo() {}
    operator K() { return key; }
    long GetUseCounter() { return UseCounter; }
};

template<typename Trait>
class CBTreePage {
    template<typename T> friend class BTree;

    using keyType    = typename Trait::keyType;
    using ObjIDType  = typename Trait::ObjIDType;
    using BTPage     = CBTreePage<Trait>;
    using ObjectInfo = tagObjectInfo<keyType, ObjIDType>;  // dos params 

public:
    CBTreePage(int maxKeys, bool unique = true);
    virtual ~CBTreePage();

    bt_ErrorCode Insert(const keyType& key, const ObjIDType ObjID);
    bt_ErrorCode Remove(const keyType& key, const ObjIDType ObjID);
    bool         Search(const keyType& key, long& ObjID);
    void         Print(ostream& os);

    // ForEach variadic — lambda recibe (ObjectInfo&, int level, args...)
    template<typename Func, typename... Args>
    void ForEach(Func func, int level, Args&&... args) {
        for (auto i = 0; i < m_KeyCount; i++) {
            if (m_SubPages[i])
                m_SubPages[i]->ForEach(func, level + 1, forward<Args>(args)...);
            func(m_Keys[i], level, forward<Args>(args)...);
        }
        if (m_SubPages[m_KeyCount])
            m_SubPages[m_KeyCount]->ForEach(func, level + 1, forward<Args>(args)...);
    }

    // FirstThat variadic — retorna primer ObjectInfo que cumple la condicion
    template<typename Func, typename... Args>
    ObjectInfo* FirstThat(Func func, int level, Args&&... args) {
        for (auto i = 0; i < m_KeyCount; i++) {
            if (m_SubPages[i]) {
                auto pTmp = m_SubPages[i]->FirstThat(func, level + 1, forward<Args>(args)...);
                if (pTmp) return pTmp;
            }
            if (func(m_Keys[i], level, forward<Args>(args)...))
                return &m_Keys[i];
        }
        if (m_SubPages[m_KeyCount]) {
            auto pTmp = m_SubPages[m_KeyCount]->FirstThat(func, level + 1, forward<Args>(args)...);
            if (pTmp) return pTmp;
        }
        return nullptr;
    }

protected:
    int  m_MinKeys, m_MaxKeys, m_MaxKeysForChilds;
    bool m_Unique, m_isRoot;
    vector<ObjectInfo> m_Keys;
    vector<BTPage*>    m_SubPages;
    int  m_KeyCount;

    void Create(); void Reset();
    void Destroy() { Reset(); delete this; }
    void clear()   { m_KeyCount = 0; }

    bool Redistribute1(int& pos); bool Redistribute2(int pos);
    void RedistributeR2L(int pos); void RedistributeL2R(int pos);
    bool TreatUnderflow(int& pos) { return Redistribute1(pos) || Redistribute2(pos); }
    bt_ErrorCode Merge(int pos);  bt_ErrorCode MergeRoot();
    void SplitChild(int pos);
    ObjectInfo& GetFirstObjectInfo();
    bool Overflow()        { return m_KeyCount > m_MaxKeys; }
    bool Underflow()       { return m_KeyCount < MinNumberOfKeys(); }
    bool IsFull()          { return m_KeyCount >= m_MaxKeys; }
    int  MinNumberOfKeys() { return 2 * m_MaxKeys / 3.0; }
    int  GetFreeCells()    { return m_MaxKeys - m_KeyCount; }
    int& NumberOfKeys()    { return m_KeyCount; }
    int  GetNumberOfKeys() { return m_KeyCount; }
    bool IsRoot()          { return m_MaxKeysForChilds != m_MaxKeys; }
    void SetMaxKeysForChilds(int order) { m_MaxKeysForChilds = order; }
    int  GetFreeCellsOnLeft(int pos);
    int  GetFreeCellsOnRight(int pos);

private:
    bool SplitRoot();
    void SplitPageInto3(vector<ObjectInfo>&, vector<BTPage*>&,
                        BTPage*&, BTPage*&, BTPage*&,
                        ObjectInfo&, ObjectInfo&);
    void MovePage(BTPage*, vector<ObjectInfo>&, vector<BTPage*>&);
};

// ── Helpers ────
template<typename Container, typename ObjType>
int binary_search(Container& c, int first, int last, ObjType& obj) {
    if (first >= last) return first;
    while (first < last) {
        auto mid = (first + last) / 2;
        if (obj == (ObjType)c[mid]) return mid;
        if (obj >  (ObjType)c[mid]) first = mid + 1;
        else                         last  = mid;
    }
    return (obj <= (ObjType)c[first]) ? first : last;
}

template<typename Container, typename ObjType>
void insert_at(Container& c, const ObjType& obj, int pos) {
    auto sz = (int)c.size();
    for (auto i = sz - 2; i >= pos; i--) c[i+1] = c[i];
    c[pos] = obj;
}

template<typename Container>
void remove(Container& c, int pos) {
    auto sz = (int)c.size();
    for (auto i = pos + 1; i < sz; i++) c[i-1] = c[i];
}

// ── Implementaciones ─────
template<typename T>
CBTreePage<T>::CBTreePage(int maxKeys, bool unique)
    : m_MaxKeys(maxKeys), m_Unique(unique), m_KeyCount(0) {
    Create(); SetMaxKeysForChilds(m_MaxKeys);
}
template<typename T> CBTreePage<T>::~CBTreePage() { Reset(); }

template<typename T>
void CBTreePage<T>::Create() {
    Reset();
    m_Keys.resize(m_MaxKeys + 1);
    m_SubPages.resize(m_MaxKeys + 2, nullptr);
    m_KeyCount = 0; m_MinKeys = 2 * m_MaxKeys / 3;
}

template<typename T>
void CBTreePage<T>::Reset() {
    for (auto i = 0; i < m_KeyCount; i++) delete m_SubPages[i];
    clear();
}

template<typename T>
bt_ErrorCode CBTreePage<T>::Insert(const keyType& key, const ObjIDType ObjID) {
    auto pos = binary_search(m_Keys, 0, m_KeyCount, key);
    if (pos < m_KeyCount && (keyType)m_Keys[pos] == key && m_Unique) return bt_duplicate;
    if (!m_SubPages[pos]) {
        ::insert_at(m_Keys, ObjectInfo(key, ObjID), pos);
        NumberOfKeys()++;
        return Overflow() ? bt_overflow : bt_ok;
    }
    auto error = m_SubPages[pos]->Insert(key, ObjID);
    if (error == bt_overflow) {
        if (!Redistribute1(pos)) SplitChild(pos);
        if (Overflow()) return bt_overflow;
        return bt_ok;
    }
    return Overflow() ? bt_overflow : bt_ok;
}

template<typename T>
bool CBTreePage<T>::Search(const keyType& key, long& ObjID) {
    auto pos = binary_search(m_Keys, 0, m_KeyCount, key);
    if (pos >= m_KeyCount) {
        if (m_SubPages[pos]) return m_SubPages[pos]->Search(key, ObjID);
        return false;
    }
    if (key == m_Keys[pos].key) { ObjID = m_Keys[pos].ObjID; m_Keys[pos].UseCounter++; return true; }
    if (key < m_Keys[pos].key && m_SubPages[pos]) return m_SubPages[pos]->Search(key, ObjID);
    return false;
}

template<typename T>
void CBTreePage<T>::Print(ostream& os) {
    // Print usa ForEach con lambda — sin punteros a funcion ni void*
    ForEach([&os](auto& info, auto level) {
        for (auto i = 0; i < level; i++) os << "\t";
        os << info.key << "->" << info.ObjID << "\n";
    }, 0);
}

template<typename T>
bool CBTreePage<T>::Redistribute1(int& pos) {
    if (m_SubPages[pos]->Underflow()) {
        auto nkol = 0, nkor = 0;
        if (pos > 0) nkol = m_SubPages[pos-1]->NumberOfKeys();
        if (pos < NumberOfKeys()) nkor = m_SubPages[pos+1]->NumberOfKeys();
        if (nkol > nkor) {
            if (m_SubPages[pos-1]->NumberOfKeys() > m_SubPages[pos-1]->MinNumberOfKeys()) RedistributeL2R(pos-1);
            else return (pos == NumberOfKeys()) ? (--pos, false) : false;
        } else {
            if (m_SubPages[pos+1]->NumberOfKeys() > m_SubPages[pos+1]->MinNumberOfKeys()) RedistributeR2L(pos+1);
            else return (pos == 0) ? (++pos, false) : false;
        }
    } else {
        auto fcol = GetFreeCellsOnLeft(pos), fcor = GetFreeCellsOnRight(pos);
        if (!fcol && !fcor && m_SubPages[pos]->IsFull()) return false;
        if (fcol > fcor) RedistributeR2L(pos); else RedistributeL2R(pos);
    }
    return true;
}

template<typename T>
bool CBTreePage<T>::Redistribute2(int pos) {
    assert(pos > 0 && pos < NumberOfKeys());
    if (m_SubPages[pos-1]->Underflow()) {
        RedistributeR2L(pos+1); RedistributeR2L(pos);
        if (m_SubPages[pos-1]->Underflow()) return false;
    } else if (m_SubPages[pos+1]->Underflow()) {
        RedistributeL2R(pos-1); RedistributeL2R(pos);
        if (m_SubPages[pos+1]->Underflow()) return false;
    } else {
        RedistributeL2R(pos-1); RedistributeR2L(pos+1);
        if (m_SubPages[pos]->Underflow()) return false;
    }
    return true;
}

template<typename T>
void CBTreePage<T>::RedistributeR2L(int pos) {
    auto pSrc = m_SubPages[pos], pDst = m_SubPages[pos-1];
    while (pSrc->GetNumberOfKeys() > pSrc->MinNumberOfKeys() &&
           pDst->GetNumberOfKeys() < pSrc->GetNumberOfKeys()) {
        ::insert_at(pDst->m_Keys, m_Keys[pos-1], pDst->NumberOfKeys()++);
        ::insert_at(pDst->m_SubPages, pSrc->m_SubPages[0], pDst->NumberOfKeys());
        m_Keys[pos-1] = pSrc->m_Keys[0];
        ::remove(pSrc->m_Keys, 0); ::remove(pSrc->m_SubPages, 0);
        pSrc->NumberOfKeys()--;
    }
}

template<typename T>
void CBTreePage<T>::RedistributeL2R(int pos) {
    auto pSrc = m_SubPages[pos], pDst = m_SubPages[pos+1];
    while (pSrc->GetNumberOfKeys() > pSrc->MinNumberOfKeys() &&
           pDst->GetNumberOfKeys() < pSrc->GetNumberOfKeys()) {
        ::insert_at(pDst->m_Keys, m_Keys[pos], 0);
        ::insert_at(pDst->m_SubPages, pSrc->m_SubPages[pSrc->NumberOfKeys()], 0);
        pDst->NumberOfKeys()++;
        m_Keys[pos] = pSrc->m_Keys[pSrc->NumberOfKeys()-1];
        pSrc->NumberOfKeys()--;
    }
}

template<typename T>
void CBTreePage<T>::SplitChild(int pos) {
    BTPage *p1 = nullptr, *p2 = nullptr;
    if (pos > 0 && m_SubPages[pos-1]->IsFull()) { p1 = m_SubPages[pos-1]; p2 = m_SubPages[pos--]; }
    if (pos < GetNumberOfKeys() && m_SubPages[pos+1]->IsFull()) { p1 = m_SubPages[pos]; p2 = m_SubPages[pos+1]; }
    vector<ObjectInfo> tmp; vector<BTPage*> tmpP;
    MovePage(p1, tmp, tmpP); tmp.push_back(m_Keys[pos]); MovePage(p2, tmp, tmpP);
    BTPage* p3 = nullptr; ObjectInfo oi1, oi2;
    SplitPageInto3(tmp, tmpP, p1, p2, p3, oi1, oi2);
    m_Keys[pos] = oi1; m_SubPages[pos] = p1;
    ::insert_at(m_Keys, oi2, pos+1); ::insert_at(m_SubPages, p2, pos+1);
    NumberOfKeys()++; m_SubPages[pos+2] = p3;
}

template<typename T>
void CBTreePage<T>::SplitPageInto3(vector<ObjectInfo>& tmp, vector<BTPage*>& tmpP,
                                    BTPage*& p1, BTPage*& p2, BTPage*& p3,
                                    ObjectInfo& oi1, ObjectInfo& oi2) {
    assert(tmp.size() >= 8); assert(tmpP.size() >= 9);
    if (!p1) p1 = new BTPage(m_MaxKeysForChilds, m_Unique);
    p1->clear();
    auto nk = (int)(tmp.size()-2)/3, i = 0;
    for (; i < nk; i++) { p1->m_Keys[i] = tmp[i]; p1->m_SubPages[i] = tmpP[i]; p1->NumberOfKeys()++; }
    p1->m_SubPages[i] = tmpP[i]; oi1 = tmp[i++];
    if (!p2) p2 = new BTPage(m_MaxKeysForChilds, m_Unique);
    p2->clear();
    nk += (int)(tmp.size()-2)/3 + 1;
    auto j = 0;
    for (; i < nk; i++, j++) { p2->m_Keys[j] = tmp[i]; p2->m_SubPages[j] = tmpP[i]; p2->NumberOfKeys()++; }
    p2->m_SubPages[j] = tmpP[i]; oi2 = tmp[i++];
    if (!p3) p3 = new BTPage(m_MaxKeysForChilds, m_Unique);
    p3->clear();
    nk = (int)tmp.size();
    for (j = 0; i < nk; i++, j++) { p3->m_Keys[j] = tmp[i]; p3->m_SubPages[j] = tmpP[i]; p3->NumberOfKeys()++; }
    p3->m_SubPages[j] = tmpP[i];
}

template<typename T>
bool CBTreePage<T>::SplitRoot() {
    BTPage *p1 = nullptr, *p2 = nullptr, *p3 = nullptr; ObjectInfo oi1, oi2;
    SplitPageInto3(m_Keys, m_SubPages, p1, p2, p3, oi1, oi2); clear();
    m_Keys[0] = oi1; m_SubPages[0] = p1; NumberOfKeys()++;
    m_Keys[1] = oi2; m_SubPages[1] = p2; NumberOfKeys()++; m_SubPages[2] = p3;
    return true;
}

template<typename T>
bt_ErrorCode CBTreePage<T>::Remove(const keyType& key, const ObjIDType ObjID) {
    auto error = bt_ok;
    auto pos = binary_search(m_Keys, 0, m_KeyCount, key);
    if (pos < NumberOfKeys() && key == m_Keys[pos].key) {
        if (!m_SubPages[pos+1]) {
            ::remove(m_Keys, pos); NumberOfKeys()--;
            return Underflow() ? bt_underflow : bt_ok;
        }
        ObjectInfo& rFirst = m_SubPages[pos+1]->GetFirstObjectInfo();
        swap(m_Keys[pos], rFirst);
        error = m_SubPages[++pos]->Remove(key, ObjID);
    } else if (pos == NumberOfKeys())
        error = m_SubPages[pos]->Remove(key, ObjID);
    else if (key <= m_Keys[pos].key) {
        if (m_SubPages[pos]) error = m_SubPages[pos]->Remove(key, ObjID);
        else return bt_nofound;
    }
    if (error == bt_underflow) {
        if (TreatUnderflow(pos)) return bt_ok;
        if (IsRoot() && NumberOfKeys() == 2) return MergeRoot();
        return Merge(pos);
    }
    return (error == bt_nofound) ? bt_nofound : bt_ok;
}

template<typename T>
bt_ErrorCode CBTreePage<T>::Merge(int pos) {
    vector<ObjectInfo> tmp; vector<BTPage*> tmpP;
    auto p1 = m_SubPages[pos-1], p2 = m_SubPages[pos], p3 = m_SubPages[pos+1];
    MovePage(p1, tmp, tmpP); tmp.push_back(m_Keys[pos-1]);
    MovePage(p2, tmp, tmpP); tmp.push_back(m_Keys[pos]);
    MovePage(p3, tmp, tmpP); p3->Destroy();
    auto nk = p1->GetFreeCells(), i = 0;
    for (; i < nk; i++) { p1->m_Keys[i] = tmp[i]; p1->m_SubPages[i] = tmpP[i]; p1->NumberOfKeys()++; }
    p1->m_SubPages[i] = tmpP[i]; m_Keys[pos-1] = tmp[i]; m_SubPages[pos-1] = p1;
    ::remove(m_Keys, pos); ::remove(m_SubPages, pos); NumberOfKeys()--;
    nk = p2->GetFreeCells(); auto j = ++i;
    for (i = 0; i < nk; i++, j++) { p2->m_Keys[i] = tmp[j]; p2->m_SubPages[i] = tmpP[j]; p2->NumberOfKeys()++; }
    p2->m_SubPages[i] = tmpP[j]; m_SubPages[pos] = p2;
    return Underflow() ? bt_underflow : bt_ok;
}

template<typename T>
bt_ErrorCode CBTreePage<T>::MergeRoot() {
    auto pos = 1;
    auto p1 = m_SubPages[pos-1], p2 = m_SubPages[pos], p3 = m_SubPages[pos+1];
    auto nk = p1->NumberOfKeys() + p2->NumberOfKeys() + p3->NumberOfKeys() + 2;
    vector<ObjectInfo> tmp; vector<BTPage*> tmpP;
    MovePage(p1, tmp, tmpP); tmp.push_back(m_Keys[pos-1]);
    MovePage(p2, tmp, tmpP); tmp.push_back(m_Keys[pos]);
    MovePage(p3, tmp, tmpP); clear();
    auto i = 0;
    for (; i < nk; i++) { m_Keys[i] = tmp[i]; m_SubPages[i] = tmpP[i]; NumberOfKeys()++; }
    m_SubPages[i] = tmpP[i];
    p1->Destroy(); p2->Destroy(); p3->Destroy();
    return bt_rootmerged;
}

template<typename T>
typename CBTreePage<T>::ObjectInfo& CBTreePage<T>::GetFirstObjectInfo() {
    if (m_SubPages[0]) return m_SubPages[0]->GetFirstObjectInfo();
    return m_Keys[0];
}

template<typename T>
void CBTreePage<T>::MovePage(BTPage* p, vector<ObjectInfo>& tmp, vector<BTPage*>& tmpP) {
    auto nk = p->GetNumberOfKeys(), i = 0;
    for (; i < nk; i++) { tmp.push_back(p->m_Keys[i]); tmpP.push_back(p->m_SubPages[i]); }
    tmpP.push_back(p->m_SubPages[i]); p->clear();
}

template<typename T> int CBTreePage<T>::GetFreeCellsOnLeft(int pos)  { return (pos > 0) ? m_SubPages[pos-1]->GetFreeCells() : 0; }
template<typename T> int CBTreePage<T>::GetFreeCellsOnRight(int pos) { return (pos < GetNumberOfKeys()) ? m_SubPages[pos+1]->GetFreeCells() : 0; }

#endif // CBTreePage_H