#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include "BTree.h"
#include "traits.h"
#include "../types.h"

using namespace std;

const TypeBTree * keys1 = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
const TypeBTree * keys2 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const TypeBTree * keys3 = "acefhjlmoqstwyz";

const T1 BTreeSize = 3;

void BTreeDemo()
{
    cout << " Demo de BTree " << endl;

    // Le pasamos correctamente el nodo contenedor 'tagObjectInfo<TypeBTree, Ref>' al Trait
    using MyTreeTrait = AscendingTrait<tagObjectInfo<TypeBTree, Ref>>;
    BTree<MyTreeTrait> bt(BTreeSize);

   
    for (T1 i = 0; keys1[i]; i++) {
        bt.Insert(keys1[i], i * i);
    }

    cout << "\nEstructura del Arbol Impresa con Lambda:" << endl;
    bt.Print(cout); // recorrido indentado por nivel, via ForEach

    cout << "\nProbando consultas seguras:" << endl;
    for (T1 i = 0; keys2[i] && i < 10; i++)
    {
        cout << "Buscando llave '" << keys2[i] << "': ";
        Ref ObjID = bt.Search(keys2[i]);
        if (ObjID != 0) 
            cout << "Encontrado -> ID = " << ObjID << endl;
        else
            cout << "No registrado." << endl;
    }

    // Iterador forward: recorre las claves de menor a mayor
    cout << "\nIteracion forward: ";
    for (auto it = bt.begin(); it != bt.end(); ++it) cout << it->key << " ";
    cout << endl;

    // Iterador backward: recorre las claves de mayor a menor
    cout << "Iteracion backward: ";
    for (auto it = bt.rbegin(); it != bt.rend(); ++it) cout << it->key << " ";
    cout << endl;

    // Remove: quita las claves de prueba y confirma que el arbol sigue sano
    cout << "\nRemoviendo claves de prueba (" << keys3 << "):" << endl;
    for (T1 i = 0; keys3[i]; i++) {
        bool ok = bt.Remove(keys3[i], -1);
        cout << "  Remove('" << keys3[i] << "') -> " << (ok ? "ok" : "no encontrada") << endl;
    }
    cout << "Elementos tras remover: " << bt.size() << ", Altura: " << bt.height() << endl;

    // operator<<: serializa el arbol 
    cout << "\noperator<<: " << bt << endl;

    // operator>>: relee ese texto 
    ostringstream oss; oss << bt;
    BTree<MyTreeTrait> bt2(BTreeSize);
    istringstream iss(oss.str());
    iss >> bt2;
    cout << "operator>> (releido): " << bt2 << endl;

    cout << "\nPropiedades: Elementos = " << bt.size() << ", Altura = " << bt.height() << endl;
}