#ifndef __AVL_H__
#define __AVL_H__

#include "BinaryTree.h"

// AVLNode: extiende BinaryTreeNode agregando m_height
template<typename T>
struct AVLNode : public BinaryTreeNode<T, AVLNode<T>> {
    using value_type = T;
    int m_height;
    AVLNode(T data, Ref ref) : BinaryTreeNode<T, AVLNode<T>>(data, ref), m_height(1) {}
};

template <typename T>
struct AscendingAVLTrait  : public BaseTrait<AVLNode<T>, less<T>> {};
template <typename T>
struct DescendingAVLTrait : public BaseTrait<AVLNode<T>, greater<T>> {};

template<typename Trait>
class AVL : public BinaryTree<Trait> {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;

private:
    int  height(Node* n)       const { return n ? n->m_height : 0; }
    void update_height(Node* n)      { if (n) n->m_height = 1 + max(height(n->m_pChild[0]), height(n->m_pChild[1])); }
    int  balance_factor(Node* n) const { return n ? height(n->m_pChild[0]) - height(n->m_pChild[1]) : 0; }

    void rotate_right(Node*& y) {
        Node* x = y->m_pChild[0];
        y->m_pChild[0] = x->m_pChild[1];
        x->m_pChild[1] = y;
        update_height(y); update_height(x); y = x;
    }
    void rotate_left(Node*& x) {
        Node* y = x->m_pChild[1];
        x->m_pChild[1] = y->m_pChild[0];
        y->m_pChild[0] = x;
        update_height(x); update_height(y); x = y;
    }
    void rebalance(Node*& n) {
        update_height(n);
        int bf = balance_factor(n);
        if      (bf >  1 && balance_factor(n->m_pChild[0]) >= 0) rotate_right(n);                               // LL
        else if (bf >  1 && balance_factor(n->m_pChild[0]) <  0) { rotate_left(n->m_pChild[0]); rotate_right(n); } // LR
        else if (bf < -1 && balance_factor(n->m_pChild[1]) <= 0) rotate_left(n);                                // RR
        else if (bf < -1 && balance_factor(n->m_pChild[1]) >  0) { rotate_right(n->m_pChild[1]); rotate_left(n); } // RL
    }

protected:
    void internal_insert(Node*& pNode, const value_type& data, Ref ref) override {
        if (!pNode) { pNode = new AVLNode<value_type>(data, ref); return; }
        internal_insert(pNode->m_pChild[!this->m_comp(data, pNode->m_data)], data, ref);
        rebalance(pNode);
    }
    Node* internal_copy(Node* pNode) override {
        if (!pNode) return nullptr;
        auto* n = new AVLNode<value_type>(pNode->m_data, pNode->m_ref);
        n->m_height    = pNode->m_height;
        n->m_pChild[0] = internal_copy(pNode->m_pChild[0]);
        n->m_pChild[1] = internal_copy(pNode->m_pChild[1]);
        return n;
    }

public:
    AVL() : BinaryTree<Trait>() {}
    AVL(const AVL& other) : BinaryTree<Trait>() {
        shared_lock<shared_mutex> lock(other.m_mtx);
        this->m_pRoot = internal_copy(other.m_pRoot);
    }
    AVL(AVL&& other) : BinaryTree<Trait>(move(other)) {}
    AVL& operator=(const AVL& other) {
        if (this != &other) { this->clear(); shared_lock<shared_mutex> lock(other.m_mtx); this->m_pRoot = internal_copy(other.m_pRoot); }
        return *this;
    }
    AVL& operator=(AVL&& other) { BinaryTree<Trait>::operator=(move(other)); return *this; }
    virtual ~AVL() {}

    int height()  const { shared_lock<shared_mutex> lock(this->m_mtx); return height(this->m_pRoot); }
    int balance() const { shared_lock<shared_mutex> lock(this->m_mtx); return balance_factor(this->m_pRoot); }
};

#endif // __AVL_H__