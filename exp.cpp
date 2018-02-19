#include <iostream>
#include <mpi/mpi.h>
#include "MyDouble.hpp"


int main(int argc, char *argv[]) {
  MPI::Init(argc, argv);
  MyDouble<52> x(125.198309412377777777);
  std::cout << x.bin() <<"\t" << x.dec();
  MPI::Finalize();
  return 0;
}