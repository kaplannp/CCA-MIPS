#include <iostream>
#include <bitset>

using namespace std;

int main(){
  unsigned int x1 = -1;
  unsigned int x2 = 4;
  unsigned long resLong = x1*x2;
  unsigned int resInt = x1*x2;
  cout << "raw " << x1*x2 << endl;
  cout << "long " << resLong << endl;
  cout << "int " << resInt << endl;
}
