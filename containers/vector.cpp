#include <cstddef>
#include <iostream>
#include <string>
#include <fstream>
#include "vector.h"

using namespace std;

void AddOne(int& n){
    n+= 2;
}

template <typename T>
void Add(T& n, T value){
    n += value;
}

template <typename T>
void PrintX(T &n, ostream &os, string sep){
    os << n << sep;
}

void DemoVector(){
    Vector<T1> v1(3);
    v1.push_back(1);
    v1.push_back(2);
    v1.push_back(-1);
    v1.push_back(4);
    cout << v1.toString() << endl;
    cout << v1 << endl;
    // cout << "hola" << 5 << endl;
    // cout.operator<<("hola")
    // ==============
    //           cout << 5 << endl;
    //           =========
    //                cout << endl;

    Vector<string> v2(10);
    v2.push_back("Hola");
    v2.push_back("Mundo");
    v2.push_back("!");
    cout << v2 << endl;
    cout << v2.toString() << endl;

    ofstream of("temp.txt");
    of << v1 << endl;
    of << v2 << endl;
    // of.close();

    ForEach(v1, AddOne);
    Print(v1, cout);
    ForEach(v1, Add<T1>, 10);
    cout << "Imprimiendo con Print enviandole el cout" << endl;
    ForEach(v1, PrintX<T1>, cout, ",");
    cout << endl;
    ForEach(v1, PrintX<T1>, cout, " ");
    cout << endl;
    Print(v1, cout);

    Print(v2, cout);
    ForEach(v2, Add<string>, "XYZ");
    cout << v2 << endl;

    Print(v1, of);
    Print(v2, of);
}