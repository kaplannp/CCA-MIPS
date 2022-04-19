#include <iostream>
#include <bitset>

using namespace std;

void func(void** p){
  *p = (void*)5l;
}

int main(){
  void* p = (void*)4l;
  cout << p << endl;
  func(&p);
  cout << p << endl;
}
