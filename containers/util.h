
#include <ostream>
using namespace std;

template <typename Container>
void Print(Container& c, ostream &os){
    os << c << endl;
}