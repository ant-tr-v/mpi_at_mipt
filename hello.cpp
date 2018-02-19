#include <iostream>
#include <mpi/mpi.h>

int main(int argc, char *argv[]) {
  MPI::Init(argc, argv);
  std::cout <<  "prs # "<< MPI::COMM_WORLD.Get_rank()<<": Hello wold!";
  if (MPI::COMM_WORLD.Get_rank() == MPI::COMM_WORLD.Get_size() - 1)
    std::cout << "- The last but the least)";
  std::cout << std::endl;
  MPI::Finalize();
  return 0;
}