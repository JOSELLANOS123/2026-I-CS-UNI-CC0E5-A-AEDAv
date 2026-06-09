#include <iostream>
#include <fstream>
#include "../types.h"
#include "heap.h"
using namespace std;

using MinH = Heap<MinHeapTrait<T1>>;
using MaxH = Heap<MaxHeapTrait<T1>>;

void DemoHeap() {
    cout << "\n=== Heap ===\n";

    // MinHeap: el menor elemento siempre en la cima
    cout << "\n[MinHeap]\n";
    MinH minH;
    for (MinH::value_type v : {5,3,8,1,4,9,2}) minH.insert(v, v*10);
    cout << "  interno: " << minH << "\n";
    auto [pv, pr] = minH.peek();
    cout << "  peek: (" << pv << "," << pr << ")\n";
    cout << "  extrayendo: ";
    while (!minH.isEmpty()) {
        auto [v, r] = minH.extract();
        cout << v << " ";
    }
    cout << "\n";

    // MaxHeap: el mayor elemento siempre en la cima
    cout << "\n[MaxHeap]\n";
    MaxH maxH;
    for (MaxH::value_type v : {5,3,8,1,4,9,2}) maxH.insert(v, v*10);
    cout << "  interno: " << maxH << "\n";
    auto [pv2, pr2] = maxH.peek();
    cout << "  peek: (" << pv2 << "," << pr2 << ")\n";
    cout << "  extrayendo: ";
    while (!maxH.isEmpty()) {
        auto [v, r] = maxH.extract();
        cout << v << " ";
    }
    cout << "\n";

    // operator<< y operator>> (persistencia)
    cout << "\n[operator<< y >>]\n";
    MinH h1;
    for (MinH::value_type v : {4,2,7,1}) h1.insert(v, v*5);
    { ofstream os("heap.txt"); os << h1; }
    MinH h2; { ifstream is("heap.txt"); is >> h2; }
    cout << "  escrito: " << h1 << "\n";
    cout << "  leido:   " << h2 << "\n";

    // Copy constructor
    MinH hCopy(h1);
    cout << "\n[copy ctor] " << hCopy << "\n";

    // Move constructor
    MinH hMove(move(hCopy));
    cout << "[move ctor] movido: " << hMove << "  vaciado size=" << hCopy.size() << "\n";
}