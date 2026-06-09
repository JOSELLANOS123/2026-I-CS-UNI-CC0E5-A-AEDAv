#include <iostream>
#include <fstream>
#include "../types.h"
#include "hashtable.h"
using namespace std;

using HT = HashTable<AscendingAVLTrait<T1>>;

void DemoHashTable() {
    cout << "\n=== HashTable (AVL) ===\n";

    HT m;

    // n[key] = value
    m[1]  = 100;
    m[5]  = 500;
    m[3]  = 300;
    m[8]  = 800;
    m[2]  = 200;
    m[5]  = 999;   
    cout << "  tabla: " << m << "\n";
    cout << "  size:  " << m.size() << "\n";

    // for (const auto& [key, value] : m)
    cout << "  iteracion: ";
    for (const auto& [key, value] : m)
        cout << key << "->" << value << " ";
    cout << "\n";

    // operator<< y operator>> (persistencia)
    { ofstream os("hash.txt"); os << m; }
    HT m2; { ifstream is("hash.txt"); is >> m2; }
    cout << "\n  escrito: " << m << "\n";
    cout << "  leido:   " << m2 << "\n";

    // Copy constructor
    HT mCopy(m);
    cout << "\n[copy ctor] " << mCopy << "\n";

    // Move constructor
    HT mMove(move(mCopy));
    cout << "[move ctor] movido: " << mMove << "  vaciado size=" << mCopy.size() << "\n";
}