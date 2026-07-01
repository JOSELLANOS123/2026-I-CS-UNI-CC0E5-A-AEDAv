#ifndef __TRAITS_H__
#define __TRAITS_H__

#include <functional> 

template <typename _Node, typename _Comp>
struct BaseTrait {
    using Node       = _Node;
    using value_type = typename _Node::value_type;
    using Comp       = _Comp;
};

template <typename _Node>
struct AscendingTrait : public BaseTrait<_Node, std::less<typename _Node::value_type>> {
};

template <typename _Node>
struct DescendingTrait : public BaseTrait<_Node, std::greater<typename _Node::value_type>> {
};

//  TRAIT ESPECÍFICO PARA BTREE
template<typename _Key, typename _ObjID = long, typename _Comp = std::less<_Key>>
struct BTreeTrait {
    using keyType    = _Key;
    using ObjIDType  = _ObjID;
    using value_type = _Key; 
    using Comp       = _Comp; // Requisito crucial para el Árbol B
};

using CharBTreeTrait = BTreeTrait<char>;

#endif // __TRAITS_H__