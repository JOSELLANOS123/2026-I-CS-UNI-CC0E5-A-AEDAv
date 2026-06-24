#include <iostream>
#include "BTree.h"
using namespace std;


using BT = BTree<BTreeTrait<TypeBTree>>;

void BTreeDemo() {
    cout << "\n=== BTree ===\n";

    const TypeBTree* keys1 = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
    constexpr auto BTreeSize = 3;

    BT bt(BTreeSize);
    for (auto i = 0; keys1[i]; i++)
        bt.Insert(keys1[i], (Ref)(i * i));

    // Print
    cout << "\n[Print]\n";
    bt.Print(cout);

    // ForEach variadic sin args extra 
    cout << "\n[ForEach - claves en orden]\n";
    bt.ForEach([](auto& info, auto level) {
        cout << info.key << " ";
    });
    cout << "\n";

    // ForEach variadic con argumento extra
    cout << "\n[ForEach - con nivel visible]\n";
    bt.ForEach([](auto& info, auto level, bool mostrarNivel) {
        if (mostrarNivel) cout << "[" << level << "]";
        cout << info.key << " ";
    }, true);
    cout << "\n";

    // FirstThat variadic sin args extra
    cout << "\n[FirstThat - primera vocal]\n";
    auto* vocal = bt.FirstThat([](auto& info, auto level) {
        auto k = info.key;
        return k=='A'||k=='E'||k=='I'||k=='O'||k=='U'
             ||k=='a'||k=='e'||k=='i'||k=='o'||k=='u';
    });
    if (vocal) cout << "  Encontrada: " << vocal->key << " (ref=" << vocal->ObjID << ")\n";

    // FirstThat variadic con argumento extra
    cout << "\n[FirstThat - buscar clave especifica]\n";
    auto* encontrado = bt.FirstThat([](auto& info, auto level, TypeBTree target) {
        return info.key == target;
    }, TypeBTree('Z'));
    if (encontrado) cout << "  Z encontrada: ref=" << encontrado->ObjID << "\n";

    // Search, size, height
    cout << "\n[Search, size, height]\n";
    cout << "  Search('A') = " << bt.Search('A') << "\n";
    cout << "  size = " << bt.size() << "\n";
    cout << "  height = " << bt.height() << "\n";
}