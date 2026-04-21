#include "linkeclist.h"
#include <iostream>
using namespace std;

void LinkedListDemo(){
    LinkedList<T1, DescendingLinkedListTrait<T1>> list;
    list.insert(1, 15);
    list.insert(2, 25);
    list.insert(3, 35);
    list.insert(4, 45);
    list.insert(5, 55);
    cout << list << endl;

    list.push_front(6, 666);
    list.push_back(7, 777);
    cout << list << endl;

    list.pop_front();
    list.pop_back();
    cout << list << endl;

    cout << list[0] << endl;
}

void ListsDemo(){
    LinkedListDemo();
    
}
