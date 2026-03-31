#include <cstddef>
#include <iostream>
#include "vector.h"

using namespace std;

void DemoVector(){
    Vector<T1> v(10);
    v.push_back(1);
    v.push_back(2);
    v.push_back(-1);
    v.push_back(4);
    for(size_t i = 0; i < v.size(); ++i){
        std::cout << v.get(i) << " ";
    }
    std::cout << std::endl;


}