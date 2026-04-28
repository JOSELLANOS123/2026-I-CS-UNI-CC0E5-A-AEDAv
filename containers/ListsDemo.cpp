#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <vector>

#include "../types.h"
#include "linkedlist.h"
#include "doublelinkedlist.h"
#include "circularlinkedlist.h"
#include "circulardoublelinkedlist.h"

using namespace std;

template <typename Container>
void DemoList(Container& list, string fileName){
    list.insert(28, 15);
    list.insert(17, 25);
    list.insert(8, 35);
    list.insert(4, 45);
    list.insert(35, 55);
    cout << list << endl;
    // Grabar la lista en un archivo
    ofstream os(fileName);
    os << list << endl;
    
    // Leer la lista desde un archivo
    ifstream is(fileName);
    is >> list;
    cout << list << endl;
}


void LinkedListDemo() {
    // Move constructor
    LinkedList<DescendingLinkedListTrait<T1>> list;
    list.insert(2, 200); list.insert(1, 100); list.insert(3, 300);
    LinkedList<DescendingLinkedListTrait<T1>> moved(std::move(list));
    auto [df, rf] = moved.pop_front();
    cout << "  pop_front -> (" << df << "," << rf << ")" << endl;
 
    // Concurrencia
    cout << "\n  Concurrencia (5 hilos x 1000 push_front):" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> lconc;
    auto worker = [&lconc](int id) {
        for (int i = 0; i < 1000; i++) lconc.push_front(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "  Tamano esperado 5000: " << lconc.size()
         << " -> " << (lconc.size() == 5000 ? "EXITO" : "FALLO") << endl;
 
    // Archivos y operadores
    cout << "\n  Escritura/Lectura archivos:" << endl;
    LinkedList<AscendingLinkedListTrait<T1>>  la;  DemoList(la,  "AscLL.txt");
    LinkedList<DescendingLinkedListTrait<T1>> ld;  DemoList(ld,  "DescLL.txt");
 
    cout << "\n  operator>>  y  operator[]:" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> lop;
    stringstream ss("[(10,100),(20,200),(30,300)]");
    ss >> lop;
    cout << "  Lista: " << lop << endl;
    cout << "  [0]=" << lop[0] << "  [2]=" << lop[2] << endl;
}

void DoubleLinkedListDemo() {
    // 1. Insercion ordenada asc y desc
    cout << "  1. Insercion ordenada:" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> asc;
    asc.insert(5,50); asc.insert(1,10); asc.insert(3,30);
    asc.insert(4,40); asc.insert(2,20);
    cout << "    Asc:  " << asc  << endl;
 
    DoubleLinkedList<DescendingDLLTrait<T1>> desc;
    desc.insert(5,50); desc.insert(1,10); desc.insert(3,30);
    desc.insert(4,40); desc.insert(2,20);
    cout << "    Desc: " << desc << endl;
 
    // 2. push_front / push_back
    cout << "\n  2. push_front / push_back:" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> pl;
    pl.push_back(20,2); pl.push_back(30,3);
    pl.push_front(10,1); pl.push_front(5,0);
    cout << "    " << pl << endl;
 
    // 3. pop_front / pop_back
    cout << "\n  3. pop_front / pop_back:" << endl;
    auto [d1,r1] = pl.pop_front();
    cout << "    pop_front -> (" << d1 << "," << r1 << ") | " << pl << endl;
    auto [d2,r2] = pl.pop_back();
    cout << "    pop_back  -> (" << d2 << "," << r2 << ") | " << pl << endl;
 
    // 4. Iterador forward (bucle nativo foreach)
    cout << "\n  4. Iterador forward (range-based for):" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> it;
    it.insert(1,10); it.insert(2,20); it.insert(3,30);
    it.insert(4,40); it.insert(5,50);
    cout << "    fwd: ";
    for (auto &v : it) cout << v << " ";
    cout << endl;
 
    // 5. Iterador backward
    cout << "\n  5. Iterador backward (rbegin/rend):" << endl;
    cout << "    bwd: ";
    for (auto it2 = it.rbegin(); it2 != it.rend(); ++it2)
        cout << *it2 << " ";
    cout << endl;
 
    // 6. ForEach / ReverseForEach
    cout << "\n  6. ForEach / ReverseForEach:" << endl;
    cout << "    ForEach fwd:        ";
    it.ForEach([](T1 &v){ cout << v << " "; });
    cout << endl;
    cout << "    ReverseForEach bwd: ";
    it.ReverseForEach([](T1 &v){ cout << v << " "; });
    cout << endl;
 
    // 7. operator[]
    cout << "\n  7. operator[]:" << endl;
    cout << "    [0]=" << it[0] << " [2]=" << it[2] << " [4]=" << it[4] << endl;
 
    // 8. Mejora libre #1: contains
    cout << "\n  8. Mejora libre #1 - contains:" << endl;
    cout << "    contains(3): " << (it.contains(3) ? "si" : "no") << endl;
    cout << "    contains(9): " << (it.contains(9) ? "si" : "no") << endl;
 
    // 9. Mejora libre #2: printBackward
    cout << "\n  9. Mejora libre #2 - printBackward:" << endl;
    cout << "    "; it.printBackward(cout); cout << endl;
 
    // 10. Copy constructor
    cout << "\n  10. Copy constructor:" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> cop(it);
    cout << "    Original: " << it  << endl;
    cout << "    Copiada:  " << cop << endl;
 
    // 11. Move constructor
    cout << "\n  11. Move constructor:" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> mov(std::move(cop));
    cout << "    Moved:  " << mov << endl;
    cout << "    Copiada tras move (size=0): " << cop.size() << endl;
 
    // 12. operator<< y operator>> con archivo
    cout << "\n  12. Escritura/Lectura archivos:" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>>  fa;  DemoList(fa,  "AscDLL.txt");
    DoubleLinkedList<DescendingDLLTrait<T1>> fd;  DemoList(fd,  "DescDLL.txt");
 
    // 13. operator>> desde stream
    cout << "\n  13. operator>> desde stream:" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> sl;
    stringstream ss("[(30,3),(10,1),(20,2)]");
    ss >> sl;
    cout << "    Stream desordenado -> " << sl << endl;
 
    // 14. Concurrencia
    cout << "\n  14. Concurrencia (5 hilos x 1000 push_front):" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> cl;
    auto worker = [&cl](int id) {
        for (int i = 0; i < 1000; i++) cl.push_front(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "    Tamano esperado 5000: " << cl.size()
         << " -> " << (cl.size() == 5000 ? "EXITO" : "FALLO") << endl;
}

void CircularLinkedListDemo() {
    // 1. Insercion ordenada
    cout << "  1. Insercion ordenada:" << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> asc;
    asc.insert(3,30); asc.insert(1,10); asc.insert(5,50);
    asc.insert(2,20); asc.insert(4,40);
    cout << "    Asc:  " << asc  << endl;
    CircularLinkedList<DescendingCLLTrait<T1>> desc;
    desc.insert(3,30); desc.insert(1,10); desc.insert(5,50);
    desc.insert(2,20); desc.insert(4,40);
    cout << "    Desc: " << desc << endl;
 
    // 2. push / pop
    cout << "\n  2. push_front / push_back / pop_front / pop_back:" << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> pl;
    pl.push_back(20,2); pl.push_back(30,3);
    pl.push_front(10,1); pl.push_front(5,0);
    cout << "    Despues de pushes: " << pl << endl;
    auto [d1,r1] = pl.pop_front();
    cout << "    pop_front -> (" << d1 << "," << r1 << ") | " << pl << endl;
    auto [d2,r2] = pl.pop_back();
    cout << "    pop_back  -> (" << d2 << "," << r2 << ") | " << pl << endl;
 
    // 3. Naturaleza circular: circularForEach x2 vueltas
    cout << "\n  3. Naturaleza circular - circularForEach x2:" << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> cl;
    cl.insert(1,10); cl.insert(2,20); cl.insert(3,30);
    cout << "    Lista: " << cl << endl;
    cout << "    x2: ";
    cl.circularForEach(2, [](T1 &v){ cout << v << " "; });
    cout << endl;
 
    // 4. Iterador forward y ForEach
    cout << "\n  4. Iterador forward (range-based for) y ForEach:" << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> it;
    it.insert(1,10); it.insert(2,20); it.insert(3,30);
    it.insert(4,40); it.insert(5,50);
    cout << "    ranged-for: ";
    for (auto &v : it) cout << v << " ";
    cout << endl;
    cout << "    ForEach:    ";
    it.ForEach([](T1 &v){ cout << v << " "; });
    cout << endl;
 
    // 5. Archivos y operadores
    cout << "\n  5. Escritura/Lectura archivos:" << endl;
    CircularLinkedList<AscendingCLLTrait<T1>>  fa; DemoList(fa,  "AscCLL.txt");
    CircularLinkedList<DescendingCLLTrait<T1>> fd; DemoList(fd,  "DescCLL.txt");
 
    // 6. Concurrencia
    cout << "\n  6. Concurrencia (5 hilos x 1000 push_front):" << endl;
    CircularLinkedList<AscendingCLLTrait<T1>> lconc;
    auto worker = [&lconc](int id) {
        for (int i = 0; i < 1000; i++) lconc.push_front(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "    Tamano esperado 5000: " << lconc.size()
         << " -> " << (lconc.size() == 5000 ? "EXITO" : "FALLO") << endl;
}

void CircularDoubleLinkedListDemo() {
    // 1. Insercion ordenada
    cout << "  1. Insercion ordenada:" << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> asc;
    asc.insert(3,30); asc.insert(1,10); asc.insert(5,50);
    asc.insert(2,20); asc.insert(4,40);
    cout << "    Asc:  " << asc  << endl;
    CircularDoubleLinkedList<DescendingCDLLTrait<T1>> desc;
    desc.insert(3,30); desc.insert(1,10); desc.insert(5,50);
    desc.insert(2,20); desc.insert(4,40);
    cout << "    Desc: " << desc << endl;
 
    // 2. push / pop
    cout << "\n  2. push / pop:" << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> pl;
    pl.push_back(20,2); pl.push_back(30,3);
    pl.push_front(10,1); pl.push_front(5,0);
    cout << "    Despues de pushes: " << pl << endl;
    auto [d1,r1] = pl.pop_front();
    cout << "    pop_front -> (" << d1 << "," << r1 << ") | " << pl << endl;
    auto [d2,r2] = pl.pop_back();
    cout << "    pop_back  -> (" << d2 << "," << r2 << ") | " << pl << endl;
 
    // 3. circularForEach fwd y bwd x2
    cout << "\n  3. circularForEach fwd/bwd x2:" << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> cl;
    cl.insert(1,10); cl.insert(2,20); cl.insert(3,30);
    cout << "    Lista: " << cl << endl;
    cout << "    fwd x2: ";
    cl.circularForEach(2,  1, [](T1 &v){ cout << v << " "; });
    cout << endl;
    cout << "    bwd x2: ";
    cl.circularForEach(2, -1, [](T1 &v){ cout << v << " "; });
    cout << endl;
 
    // 4. Iteradores fwd y bwd
    cout << "\n  4. Iteradores forward y backward:" << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> it;
    it.insert(1,10); it.insert(2,20); it.insert(3,30);
    it.insert(4,40); it.insert(5,50);
    cout << "    ranged-for fwd: ";
    for (auto &v : it) cout << v << " ";
    cout << endl;
    cout << "    ranged-for bwd: ";
    for (auto it2 = it.rbegin(); it2 != it.rend(); ++it2)
        cout << *it2 << " ";
    cout << endl;
 
    // 5. ForEach / ReverseForEach
    cout << "\n  5. ForEach / ReverseForEach:" << endl;
    cout << "    ForEach fwd:        ";
    it.ForEach([](T1 &v){ cout << v << " "; });
    cout << endl;
    cout << "    ReverseForEach bwd: ";
    it.ReverseForEach([](T1 &v){ cout << v << " "; });
    cout << endl;
 
    // 6. Archivos
    cout << "\n  6. Escritura/Lectura archivos:" << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>>  fa; DemoList(fa,  "AscCDLL.txt");
    CircularDoubleLinkedList<DescendingCDLLTrait<T1>> fd; DemoList(fd,  "DescCDLL.txt");
 
    // 7. Concurrencia
    cout << "\n  7. Concurrencia (5 hilos x 1000 push_front):" << endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> lconc;
    auto worker = [&lconc](int id) {
        for (int i = 0; i < 1000; i++) lconc.push_front(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "    Tamano esperado 5000: " << lconc.size()
         << " -> " << (lconc.size() == 5000 ? "EXITO" : "FALLO") << endl;
}

void ListsDemo(){
    LinkedListDemo();
    CircularLinkedListDemo();
    DoubleLinkedListDemo();
    CircularDoubleLinkedListDemo();
}

void TestConcurrencia() {
    cout << "\nTEST DE CONCURRENCIA" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> list;

    // 5 hilos van a intentar meter 1000 elementos cada uno al mismo tiempo
    auto worker = [&list](int thread_id) {
        for(int i = 0; i < 1000; i++) {
            list.push_front(i, thread_id);
        }
    };

    thread t1(worker, 1);
    thread t2(worker, 2);
    thread t3(worker, 3);
    thread t4(worker, 4);
    thread t5(worker, 5);

    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();

    cout << "Se lanzaron 5 hilos insertando 1000 elementos simultaneamente." << endl;
    cout << "Tamano de la lista (Esperado 5000): " << list.size() << endl;
    if(list.size() == 5000) {
        cout << "ESTADO: EXITO - El shared_mutex previno condiciones de carrera perfectamente." << endl;
    } else {
        cout << "ESTADO: FALLO - Hubo corrupcion de memoria." << endl;
    }
}


void ListsDemo() {
    cout << "=== LinkedList ===" << endl;
    LinkedListDemo();
    cout << "\n=== DoubleLinkedList ===" << endl;
    DoubleLinkedListDemo();
    cout << "\n=== CircularLinkedList ===" << endl;
    CircularLinkedListDemo();
    cout << "\n=== CircularDoubleLinkedList ===" << endl;
    CircularDoubleLinkedListDemo();
    cout << "\n=== FIN ===" << endl;
}