#include <iostream>
#include <mpi/mpi.h>
#include "MyDouble.hpp"
using namespace std;

int main(int argc, char *argv[]) {
  MPI::Init(argc, argv);
  MyDouble<7000> a(1), b(1), t;

  for(int i = 2; i < 10000; ++ i) {
    t = a;
    a = a + b;
    b = t;
  }

  cout << a.dec();
  MPI::Finalize();
  return 0;
}