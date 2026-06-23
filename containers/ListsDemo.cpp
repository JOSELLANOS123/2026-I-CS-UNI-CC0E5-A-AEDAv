#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>

#include "../types.h"
#include "linkedlist.h"

using namespace std;

// ── Demo basico: insert + print + archivo ───
template <typename Container>
void DemoList(Container& list, string fileName) {
    list.insert(28, 15);
    list.insert(17, 25);
    list.insert(8,  35);
    list.insert(4,  45);
    list.insert(35, 55);
    cout << list << endl;

    ofstream os(fileName);
    os << list << endl;

    ifstream is(fileName);
    is >> list;
    cout << list << endl;
}

void LinkedListDemo() {
    LinkedList<AscendingLinkedListTrait<T1>>  list;
    DemoList(list,  "AscLL.txt");
    LinkedList<DescendingLinkedListTrait<T1>> list2;
    DemoList(list2, "DescLL.txt");
}

// ── Test de concurrencia ──
void TestConcurrencia() {
    cout << "\nTEST DE CONCURRENCIA" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> list;

    auto worker = [&list](int thread_id) {
        for (int i = 0; i < 1000; i++)
            list.push_back(i, thread_id);
    };

    thread t1(worker, 1), t2(worker, 2), t3(worker, 3),
           t4(worker, 4), t5(worker, 5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();

    cout << "Tamano de la lista (Esperado 5000): " << list.size() << endl;
    if (list.size() == 5000)
        cout << "ESTADO: EXITO\n";
    else
        cout << "ESTADO: FALLO - hubo corrupcion de memoria.\n";
}

// ── Test de operadores ─
void TestOperators() {
    cout << "\nTEST DE OPERADORES" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> list;

    list.insert(10, 100);
    list.insert(20, 200);
    list.insert(30, 300);

    cout << "operator<<: " << list << endl;
    cout << "operator[] [0]: " << list[0] << endl;
    cout << "operator[] [2]: " << list[2] << endl;
}

// ── Punto de entrada ───
void ListsDemo() {
    cout << "=== LinkedList Demo ===\n";
    LinkedListDemo();
    TestConcurrencia();
    TestOperators();
    cout << "\n=== FIN DE LAS PRUEBAS ===\n";
}