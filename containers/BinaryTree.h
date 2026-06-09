#ifndef __BINARYTREE_H__
#define __BINARYTREE_H__

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>
#include <memory>
#include <shared_mutex>
#include <mutex>
#include <utility>
#include "traits.h"
#include "../types.h"
using namespace std;

// Nodo
template <typename T, typename DerivedNode = void>
struct BinaryTreeNode {
    using value_type = T;
    using Node = conditional_t<is_void_v<DerivedNode>, BinaryTreeNode, DerivedNode>;
    T     m_data;
    Ref   m_ref;
    Node* m_pChild[2];
    BinaryTreeNode(T d, Ref r) : m_data(d), m_ref(r), m_pChild{nullptr, nullptr} {}
};

// Iteradores: comparten ownership del snapshot via shared_ptr
template<typename Node, typename value_type>
class BTForwardIterator {
    shared_ptr<vector<Node*>> m_v;
    size_t m_i;
public:
    BTForwardIterator(shared_ptr<vector<Node*>> v, size_t i) : m_v(move(v)), m_i(i) {}
    value_type&       operator*()  const { return (*m_v)[m_i]->m_data; }
    Node*             getNode()    const { return (*m_v)[m_i]; }
    BTForwardIterator& operator++()       { ++m_i; return *this; }
    bool operator==(const BTForwardIterator& o) const { return m_i == o.m_i; }
    bool operator!=(const BTForwardIterator& o) const { return m_i != o.m_i; }
};

template<typename Node, typename value_type>
class BTBackwardIterator {
    shared_ptr<vector<Node*>> m_v;
    ptrdiff_t m_i;
public:
    BTBackwardIterator(shared_ptr<vector<Node*>> v, ptrdiff_t i) : m_v(move(v)), m_i(i) {}
    value_type&        operator*()  const { return (*m_v)[m_i]->m_data; }
    Node*              getNode()    const { return (*m_v)[m_i]; }
    BTBackwardIterator& operator++()       { --m_i; return *this; }
    bool operator==(const BTBackwardIterator& o) const { return m_i == o.m_i; }
    bool operator!=(const BTBackwardIterator& o) const { return m_i != o.m_i; }
};

// TraversalView
template<typename Node, typename value_type>
class TraversalView {
    shared_ptr<vector<Node*>> m_snap;
public:
    enum class TraversalOrder { INORDER, PREORDER, POSTORDER };  // Mejora libre #3
    using ForwardIt  = BTForwardIterator<Node, value_type>;
    using BackwardIt = BTBackwardIterator<Node, value_type>;
    using View       = TraversalView<Node, value_type>;
    explicit TraversalView(vector<Node*> s) : m_snap(make_shared<vector<Node*>>(move(s))) {}
    ForwardIt  begin()  const { return {m_snap, 0}; }
    ForwardIt  end()    const { return {m_snap, m_snap->size()}; }
    BackwardIt rbegin() const { return {m_snap, (ptrdiff_t)m_snap->size() - 1}; }
    BackwardIt rend()   const { return {m_snap, (ptrdiff_t)-1}; }
    template<typename F, typename... A> void forEach (F f, A&&... a) { for(auto it=begin();  it!=end();  ++it) f(*it, forward<A>(a)...); }
    template<typename F, typename... A> void rForEach(F f, A&&... a) { for(auto it=rbegin(); it!=rend(); ++it) f(*it, forward<A>(a)...); }
};

// BinaryTree
template<typename Trait>
class BinaryTree {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    enum class TraversalOrder { INORDER, PREORDER, POSTORDER };  // Mejora libre #3
    using ForwardIt  = BTForwardIterator<Node, value_type>;
    using BackwardIt = BTBackwardIterator<Node, value_type>;
    using View       = TraversalView<Node, value_type>;




protected:
    Node*  m_pRoot = nullptr;
    Comp   m_comp;
    mutable shared_mutex m_mtx;

    virtual void internal_insert(Node*& pNode, const value_type& data, Ref ref) {
        if (!pNode) { pNode = new Node(data, ref); return; }
        internal_insert(pNode->m_pChild[!m_comp(data, pNode->m_data)], data, ref);
    }
    virtual void internal_clear(Node* pNode) {
        if (!pNode) return;
        internal_clear(pNode->m_pChild[0]);
        internal_clear(pNode->m_pChild[1]);
        delete pNode;
    }
    virtual Node* internal_copy(Node* pNode) {
        if (!pNode) return nullptr;
        Node* n = new Node(pNode->m_data, pNode->m_ref);
        n->m_pChild[0] = internal_copy(pNode->m_pChild[0]);
        n->m_pChild[1] = internal_copy(pNode->m_pChild[1]);
        return n;
    }
    virtual Node* internal_search(Node* pNode, const value_type& data) const {
        if (!pNode) return nullptr;
        if (!m_comp(data, pNode->m_data) && !m_comp(pNode->m_data, data)) return pNode;
        return internal_search(pNode->m_pChild[!m_comp(data, pNode->m_data)], data);
    }
    virtual size_t internal_size(Node* n) const {
        return n ? 1 + internal_size(n->m_pChild[0]) + internal_size(n->m_pChild[1]) : 0;
    }



    void fill(Node* n, vector<Node*>& v, TraversalOrder order) const {
        if (!n) return;
        if (order == TraversalOrder::PREORDER)  v.push_back(n);
        fill(n->m_pChild[0], v, order);
        if (order == TraversalOrder::INORDER)   v.push_back(n);
        fill(n->m_pChild[1], v, order);
        if (order == TraversalOrder::POSTORDER) v.push_back(n);
    }

    TraversalView<Node,value_type> make_view(TraversalOrder order) const {
        vector<Node*> v; fill(m_pRoot, v, order);
        return TraversalView<Node,value_type>(move(v));
    }

    void print_node(ostream& os, Node* n, const string& pre, bool right) const {
        if (!n) return;
        print_node(os, n->m_pChild[1], pre + (right ? "    " : "|   "), false);
        os << pre << (right ? "+-- " : "\\-- ") << n->m_data << "\n";
        print_node(os, n->m_pChild[0], pre + (right ? "|   " : "    "), true);
    }

    string traversalToString(TraversalOrder order) const {
        vector<Node*> v; fill(m_pRoot, v, order);
        ostringstream os; os << "["; bool first = true;
        for (auto* node : v) {
            if (!first) os << ",";
            os << "(" << node->m_data << "," << node->m_ref << ")";
            first = false;
        }
        return os.str() + "]";
    }

public:
    BinaryTree() {}

    // t3 copy constructor
    BinaryTree(const BinaryTree& other) : m_pRoot(nullptr) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = internal_copy(other.m_pRoot);
    }
    // t4 move constructor
    BinaryTree(BinaryTree&& other) : m_pRoot(nullptr) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = exchange(other.m_pRoot, nullptr);
    }
    BinaryTree& operator=(const BinaryTree& other) {
        if (this != &other) { clear(); shared_lock<shared_mutex> lock(other.m_mtx); m_pRoot = internal_copy(other.m_pRoot); }
        return *this;
    }
    BinaryTree& operator=(BinaryTree&& other) {
        if (this != &other) { clear(); unique_lock<shared_mutex> lock(other.m_mtx); m_pRoot = exchange(other.m_pRoot, nullptr); }
        return *this;
    }
    // t5 destructor seguro
    virtual ~BinaryTree() { clear(); }

    void clear() {
        unique_lock<shared_mutex> lock(m_mtx);
        internal_clear(m_pRoot); m_pRoot = nullptr;
    }

    void insert(const value_type& data, Ref ref) {
        unique_lock<shared_mutex> lock(m_mtx);
        internal_insert(m_pRoot, data, ref);
    }

    // mejora libre #1: search con mismo patron tuple que pop_front/pop_back
    tuple<value_type, Ref> search(const value_type& data) const {
        shared_lock<shared_mutex> lock(m_mtx);
        Node* n = internal_search(m_pRoot, data);
        if (!n) throw runtime_error("elemento no encontrado");
        return make_tuple(n->m_data, n->m_ref);
    }

    size_t size() const { shared_lock<shared_mutex> lock(m_mtx); return internal_size(m_pRoot); }

    // mejora libre #2: TraversalView unifica forward/backward en 3 metodos
    View inorder()   const { shared_lock<shared_mutex> lock(m_mtx); return make_view(TraversalOrder::INORDER);   }
    View preorder()  const { shared_lock<shared_mutex> lock(m_mtx); return make_view(TraversalOrder::PREORDER);  }
    View postorder() const { shared_lock<shared_mutex> lock(m_mtx); return make_view(TraversalOrder::POSTORDER); }

    // t8 range-based for — delega a inorder
    ForwardIt begin() const { return inorder().begin(); }
    ForwardIt end()   const { return inorder().end();   }

    // t9 toString — parametrizado, por defecto inorder
    string toString(TraversalOrder order = TraversalOrder::INORDER) const {
        shared_lock<shared_mutex> lock(m_mtx);
        return traversalToString(order);
    }

    // t10 operator<< — inorder, funciona con cout y ofstream sin cambios
    friend ostream& operator<<(ostream& os, const BinaryTree& tree) {
        shared_lock<shared_mutex> lock(tree.m_mtx);
        vector<Node*> v; tree.fill(tree.m_pRoot, v, TraversalOrder::INORDER);
        os << "[";
        for (size_t i = 0; i < v.size(); ++i) {
            if (i) os << ",";
            os << "(" << v[i]->m_data << "," << v[i]->m_ref << ")";
        }
        return os << "]";
    }

    // t11 operator>>
    friend istream& operator>>(istream& is, BinaryTree& tree) {
        char ch;
        if (!(is >> ch) || ch != '[') { is.clear(ios_base::failbit); return is; }
        value_type data; Ref ref; char comma, paren;
        while (is >> ch && ch != ']')
            if (ch == '(' && is >> data >> comma >> ref >> paren && comma == ',' && paren == ')')
                tree.insert(data, ref);
        return is;
    }

    void printTree(ostream& os = cout) const {
        shared_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) { os << "(arbol vacio)\n"; return; }
        print_node(os, m_pRoot, "", true);
    }
};

#endif // __BINARYTREE_H__