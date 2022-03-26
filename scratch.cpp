#include <iostream>
#include <bitset>

using namespace std;

void set3(int& x){
  x = 3;
}
int main(){
  bitset<5> x = bitset<5>(1);
  bitset<5> y = bitset<5>(1 << 4);
  cout << x << endl;
  cout << y << endl;
}
