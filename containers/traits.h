#ifndef __TRAITS_H__
#define __TRAITS_H__

#include <functional>
#include "../types.h"

// Trait base: define que tipo de nodo (Node) guarda cada elemento y con que comparador (Comp) se ordenan.
template <typename _Node, typename _Comp>
struct BaseTrait {
    using Node       = _Node;
    using value_type = typename _Node::value_type;
    using Comp       = _Comp;
};

// Orden ascendente (usa std::less).
template <typename _Node>
struct AscendingTrait : public BaseTrait<_Node, std::less<typename _Node::value_type>> {
};

// Orden descendente (usa std::greater).
template <typename _Node>
struct DescendingTrait : public BaseTrait<_Node, std::greater<typename _Node::value_type>> {
};

//  TRAIT ESPECÍFICO PARA BTREE
template<typename _Key, typename _ObjID = Ref, typename _Comp = std::less<_Key>>
struct BTreeTrait {
    using keyType    = _Key;
    using ObjIDType  = _ObjID;
    using value_type = _Key; 
    using Comp       = _Comp; // Requisito crucial para el Árbol B
};

// Trait listo para usar: BTree con claves de tipo TypeBTree (char).
using CharBTreeTrait = BTreeTrait<TypeBTree>;

#endif // __TRAITS_H__