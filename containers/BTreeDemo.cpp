#include <iostream>
#include "BTree.h"
using namespace std;

// Alias — sin tipos nativos directos en el demo
using CharBTree = BTree<AscendingBTreeTrait<char>>;
using IntBTree  = BTree<AscendingBTreeTrait<T1>>;

void BTreeDemo() {
    cout << "\n=== BTree ===\n";

    const char* keys1 = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
    constexpr auto BTreeSize = 3;

    // Insercion con char
    CharBTree bt(BTreeSize);
    for (auto i = 0; keys1[i]; i++)
        bt.Insert(keys1[i], i * i);

    cout << "\n[Print — inorder via ForEach interno]\n";
    bt.Print(cout);

    cout << "\n[ForEach variadic — muestra clave y nivel]\n";
    bt.ForEach([](auto& info, auto level) {
        cout << string(level * 2, ' ') << info.key << " (nivel " << level << ")\n";
    });

    cout << "\n[ForEach variadic con argumento extra — prefijo]\n";
    bt.ForEach([](auto& info, auto level, const string& prefix) {
        cout << prefix << info.key << "\n";
    }, string(">> "));

    cout << "\n[FirstThat variadic — buscar primera clave > 'M']\n";
    auto pResult = bt.FirstThat([](auto& info, auto level) {
        return info.key > 'M';
    });
    if (pResult)
        cout << "  Encontrado: " << pResult->key << " -> " << pResult->ObjID << "\n";

    cout << "\n[FirstThat variadic con argumento — buscar clave especifica]\n";
    auto pResult2 = bt.FirstThat([](auto& info, auto level, char target) {
        return info.key == target;
    }, 'Z');
    if (pResult2)
        cout << "  Encontrado: " << pResult2->key << " -> " << pResult2->ObjID << "\n";

    // Busqueda
    cout << "\n[Search]\n";
    auto id = bt.Search('A');
    cout << "  Search('A'): " << (id != -1 ? to_string(id) : "no encontrado") << "\n";

    // Con T1 (int)
    cout << "\n[IntBTree con T1]\n";
    IntBTree ibt(BTreeSize);
    for (IntBTree::value_type v = 1; v <= 10; ++v)
        ibt.Insert(v, v * 100);
    ibt.Print(cout);
    cout << "  size=" << ibt.size() << " height=" << ibt.height() << "\n";
}