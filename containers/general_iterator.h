#ifndef __ITERATOR_H__
#define __ITERATOR_H__
#include <algorithm>
#include <utility>

template <typename Container, class IteratorBase> // 
class general_iterator
{public:
    using Node = typename Container::Node;
    using value_type = typename Container::value_type;
    
protected:
    Container *m_pContainer;
    Node      *m_pNode;
public:
    general_iterator(Container* c, Node* n) : m_pContainer(c), m_pNode(n) {}
    general_iterator(const general_iterator& o)               // fix #1
        : m_pContainer(o.m_pContainer), m_pNode(o.m_pNode) {}
    general_iterator(general_iterator&& o)
        : m_pContainer(o.m_pContainer), m_pNode(o.m_pNode) {}

    IteratorBase& operator=(const IteratorBase& o) {
        m_pContainer = o.m_pContainer;
        m_pNode      = o.m_pNode;
        return *static_cast<IteratorBase*>(this);
    }
    Node*        getNode()  const { return m_pNode; }
    value_type&  operator*()      { return m_pNode->getDataRef(); }

    friend bool operator==(const IteratorBase& a, const IteratorBase& b) { return a.m_pNode == b.m_pNode; }
    friend bool operator!=(const IteratorBase& a, const IteratorBase& b) { return a.m_pNode != b.m_pNode; } // fix #2
    // operator++ lo define cada iterador concreto (patron CRTP del profe)

};

#endif






