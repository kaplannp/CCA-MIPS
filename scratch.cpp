#include <iostream>
#include <bitset>

using namespace std;

int main(){
  unsigned int x1 = 1;
  unsigned int x2 = 0;
  unsigned int x3 = -1;
  cout << std::bitset<32>(x2-x1) << endl;
  cout << x3 << endl;
}
