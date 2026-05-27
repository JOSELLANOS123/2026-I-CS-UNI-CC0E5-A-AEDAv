#include <iostream>
#include <fstream>
#include <thread>
#include "../types.h"
#include "BinaryTree.h"
using namespace std;

using AscTree = BinaryTree<AscendingTrait<BinaryTreeNode<T1>>>;

void BinaryTreeDemo() {
    cout << "=== BinaryTree ===\n";
    AscTree t;
    for (AscTree::value_type v : {5,3,7,1,4,6,8}) t.insert(v, v*10);

    // t9 toString
    cout << "\n[toString]\n" << t.toString() << "\n";

    // t10 operator<< consola + archivo
    cout << "\n[operator<<]\n" << t << "\n";
    { ofstream os("tree.txt"); os << t; }

    // t11 operator>>
    AscTree t2; { ifstream is("tree.txt"); is >> t2; }
    cout << "\n[operator>>] leido: " << t2 << "\n";

    // t3 copy constructor
    AscTree tCopy(t);
    cout << "\n[copy ctor] " << tCopy << "\n";

    // t4 move constructor
    AscTree tMove(move(tCopy));
    cout << "[move ctor] movido: " << tMove << "  vaciado: " << tCopy << "\n";

    // t5 destructor seguro
    { AscTree tmp; tmp.insert(99,99); }
    cout << "[destructor] OK\n";

    // t6 forward inorder
    cout << "\n[inorder fwd] ";
    for (auto& v : t.inorder()) cout << v << " ";

    // t7 backward inorder
    cout << "\n[inorder bwd] ";
    for (auto it = t.inorder().rbegin(); it != t.inorder().rend(); ++it) cout << *it << " ";

    // t8 range-based for (inorder por defecto)
    cout << "\n[foreach nativo] ";
    for (auto& v : t) cout << v << " ";

    // t13/t14 preorder
    cout << "\n[preorder  fwd] "; t.preorder().forEach ([](T1& v){ cout << v << " "; });
    cout << "\n[preorder  bwd] "; t.preorder().rForEach([](T1& v){ cout << v << " "; });

    // t15/t16 postorder
    cout << "\n[postorder fwd] "; t.postorder().forEach ([](T1& v){ cout << v << " "; });
    cout << "\n[postorder bwd] "; t.postorder().rForEach([](T1& v){ cout << v << " "; });

    // t17 search
    auto [val, ref] = t.search(4);
    cout << "\n[search(4)] dato=" << val << " ref=" << ref << "\n";

    // t12 concurrencia
    AscTree tc;
    auto worker = [&tc](int id){ for(int i=0;i<200;i++) tc.insert(i*id, id); };
    thread th1(worker,1),th2(worker,2),th3(worker,3),th4(worker,4),th5(worker,5);
    th1.join();th2.join();th3.join();th4.join();th5.join();
    cout << "[concurrencia] esperado 1000: " << tc.size() << "\n";

    // printTree
    cout << "\n[printTree]\n"; t.printTree();
}