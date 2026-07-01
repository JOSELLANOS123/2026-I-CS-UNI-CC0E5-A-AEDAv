#include <iostream>
#include <stdlib.h>
#include <string>
#include "BTree.h"
#include "traits.h"
#include "../types.h"

using namespace std;

const char * keys1 = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
const char * keys2 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

const size_t BTreeSize = 3;

void BTreeDemo()
{
    cout << "=== Mi Demo Personalizado de BTree (Hasta Concurrencia) ===" << endl;
    
    // Le pasamos correctamente el nodo contenedor nativo 'tagObjectInfo<char, long>' al Trait
    using MyTreeTrait = AscendingTrait<tagObjectInfo<char, long>>;
    BTree<MyTreeTrait> bt(BTreeSize); 
    
    for (size_t i = 0; keys1[i]; i++) {
        bt.Insert(keys1[i], i * i);
    }
    
    cout << "\nEstructura del Arbol Impresa con Lambda:" << endl;
    bt.Print(cout);

    cout << "\nProbando consultas seguras:" << endl;
    for (size_t i = 0; keys2[i] && i < 10; i++)
    {
        cout << "Buscando llave '" << keys2[i] << "': ";
        long ObjID = bt.Search(keys2[i]);
        if (ObjID != 0) 
            cout << "Encontrado -> ID = " << ObjID << endl;
        else
            cout << "No registrado." << endl;
    }
    
    cout << "\nPropiedades: Elementos = " << bt.size() << ", Altura = " << bt.height() << endl;
}