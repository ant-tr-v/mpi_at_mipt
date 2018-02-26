#include <iostream>
#include <mpi/mpi.h>
#include "MyDouble.hpp"
using namespace std;

template<uint64_t pers>
MyDouble<pers> fibonacci(int n){
  MyDouble<pers> a(1), b(1), t;
  for(int i = 2; i < n; ++ i) {
    t = a;
    a = a + b;
    b = t;
  }
  return a;
}

template<uint64_t pers>
MyDouble<pers> fact(uint n){
  MyDouble<pers> r;
  for(uint i = 1; i <= n; ++i){
    r *= MyDouble<pers>(i);
  }
  return r;
}


int main(int argc, char *argv[]) {
  MPI::Init(argc, argv);
  cout << "F(200) = " << fibonacci<110>(150).dec() <<"\n";
  //cout << MyDouble<64>::normalize(0).bin() <<" "<< MyDouble<64>::normalize(1025).bin()
  //     <<" " << MyDouble<64>::normalize(1025).dec() << " " << MyDouble<64>::normalize(1025, -4).dec() << "\n";
  cout <<"77! = "<< fact<380>(77).dec() <<"\n";
  cout << (MyDouble<32>(0.98765)/MyDouble<32>(1.23456e-7)).dec();
  MPI::Finalize();
  return 0;
}