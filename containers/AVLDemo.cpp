#include <iostream>
#include <thread>
#include "../types.h"
#include "avl.h"
using namespace std;

using AscAVL = AVL<AscendingAVLTrait<T1>>;

void AVLDemo() {
    cout << "\n=== AVL ===\n";

    // insercion ascendente (provoca rotaciones RR)
    cout << "\n[ascendente]\n";
    AscAVL t1;
    for (int i = 1; i <= 7; ++i) {
        t1.insert(i, i*10);
        cout << "  insert(" << i << ") h=" << t1.height() << " bf=" << t1.balance() << "\n";
    }
    t1.printTree();

    // insercion descendente (rotaciones LL)
    cout << "\n[descendente]\n";
    AscAVL t2;
    for (int i = 7; i >= 1; --i) {
        t2.insert(i, i*10);
        cout << "  insert(" << i << ") h=" << t2.height() << " bf=" << t2.balance() << "\n";
    }
    t2.printTree();

    // mixta — LR y RL
    cout << "\n[mixta LL/RR/LR/RL]\n";
    AscAVL t3;
    for (int v : {5,3,7,1,4,6,8,2}) {
        t3.insert(v, v*10);
        cout << "  insert(" << v << ") h=" << t3.height() << " bf=" << t3.balance() << "\n";
    }
    t3.printTree();

    // copy constructor preserva altura
    AscAVL tCopy(t1);
    cout << "\n[copy ctor] h original=" << t1.height() << " h copia=" << tCopy.height() << "\n";

    // concurrencia
    AscAVL tc;
    auto worker = [&tc](int id){ for(int i=0;i<200;i++) tc.insert(i*id, id); };
    thread th1(worker,1),th2(worker,2),th3(worker,3),th4(worker,4),th5(worker,5);
    th1.join();th2.join();th3.join();th4.join();th5.join();
    cout << "[concurrencia] size=" << tc.size() << " h=" << tc.height() << " bf=" << tc.balance() << "\n";
}