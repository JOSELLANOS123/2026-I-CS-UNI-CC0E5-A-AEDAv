#include <iostream>
#include <fstream>
#include <thread>
#include "../types.h"
#include "linkedlist.h"
#include "doublelinkedlist.h"
#include "circularlinkedlist.h"
#include "circulardoublelinkedlist.h"
using namespace std;

template<typename C>
void DemoFileIO(C& list, const string& fname) {
    { ofstream os(fname); os << list; }
    C tmp; { ifstream is(fname); is >> tmp; }
    cout << "  escrito: " << list << "\n  leido:   " << tmp << "\n";
}

void LinkedListDemo() {
    cout << "LinkedList \n";
    LinkedList<AscendingTrait<LLNode<T1>>> list;
    list.insert(3,30); list.insert(1,10); list.insert(2,20);
    cout << "  insert asc: " << list << "\n";
    DemoFileIO(list, "LL.txt");
    cout << "  iterator: ";
    for (auto it = list.begin(); it != list.end(); ++it) cout << *it << " ";
    cout << "\n";
    // concurrencia
    LinkedList<AscendingTrait<LLNode<T1>>> lc;
    auto w = [&lc](int id){ for(int i=0;i<1000;i++) lc.push_front(i,id); };
    thread t1(w,1),t2(w,2),t3(w,3),t4(w,4),t5(w,5);
    t1.join();t2.join();t3.join();t4.join();t5.join();
    cout << "  concurrencia (esperado 5000): " << lc.size() << "\n";
}

void DoubleLinkedListDemo() {
    cout << "DoubleLinkedList \n";
    DoubleLinkedList<AscendingTrait<DLLNode<T1>>> list;
    list.insert(3,30); list.insert(1,10); list.insert(5,50); list.insert(2,20); list.insert(4,40);
    cout << "  forward:  " << list << "\n";
    cout << "  backward: ";
    for (auto it = list.rbegin(); it != list.rend(); ++it) cout << *it << " ";
    cout << "\n";
    DemoFileIO(list, "DLL.txt");
}

void CircularLinkedListDemo() {
    cout << "CircularLinkedList \n";
    CircularLinkedList<AscendingTrait<LLNode<T1>>> list;
    list.insert(3,30); list.insert(1,10); list.insert(5,50);
    list.insert(2,20); list.insert(4,40);
    cout << "  operator<<: " << list << "\n";
    cout << "  cbegin/cend: ";
    for (auto it = list.cbegin(); it != list.cend(); ++it) cout << *it << " ";
    cout << "\n";
    CircularLinkedList<AscendingTrait<LLNode<T1>>> c;
    c.insert(1,10); c.insert(2,20); c.insert(3,30);
    cout << "  circularForEach x2: ";
    c.circularForEach(2, [](T1& v){ cout << v << " "; });
    cout << "\n";
    DemoFileIO(list, "CLL.txt");
}

void CircularDoubleLinkedListDemo() {
    cout << " CircularDoubleLinkedList \n";
    CircularDoubleLinkedList<AscendingTrait<DLLNode<T1>>> list;
    list.insert(3,30); list.insert(1,10); list.insert(5,50);
    list.insert(2,20); list.insert(4,40);
    cout << "  operator<<: " << list << "\n";
    cout << "  cbegin/cend:   ";
    for (auto it = list.cbegin(); it != list.cend(); ++it) cout << *it << " ";
    cout << "\n  crbegin/crend: ";
    for (auto it = list.crbegin(); it != list.crend(); ++it) cout << *it << " ";
    cout << "\n";
    CircularDoubleLinkedList<AscendingTrait<DLLNode<T1>>> c;
    c.insert(1,10); c.insert(2,20); c.insert(3,30);
    cout << "  circularForEach fwd x2: ";
    c.circularForEach(2, 1,  [](T1& v){ cout << v << " "; }); cout << "\n";
    cout << "  circularForEach bwd x2: ";
    c.circularForEach(2, -1, [](T1& v){ cout << v << " "; }); cout << "\n";
    DemoFileIO(list, "CDLL.txt");
}

void ListsDemo() {
    LinkedListDemo();
    DoubleLinkedListDemo();
    CircularLinkedListDemo();
    CircularDoubleLinkedListDemo();
}