#include <iostream>
#include <mpi/mpi.h>
#include <chrono>

int main(int argc, char *argv[]) {
  MPI::Init(argc, argv);
  auto n = MPI::COMM_WORLD.Get_size(), rank = MPI::COMM_WORLD.Get_rank();
  if(n == 1)
    std::cout << "I feel lonely and scared\n";
  else if (rank == 0) {
    int a = rank;
    auto time = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < 1000000; ++i) {
      MPI::COMM_WORLD.Send(&a, 1, MPI_INT, (rank + 1) % n, 0);
      MPI::COMM_WORLD.Recv(&a, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG);
    }
    std::cout <<  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - time).count()*1. /n <<"ns" <<std::endl;
    for(int i = 0; i < 1000000; ++i);
  }else{
    int a;
    for(int i = 0; i < 1000000; ++i) {
      MPI::COMM_WORLD.Recv(&a, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG);

      std::cout << "[" << rank << "] Received message from " << a << std::endl;
      MPI::COMM_WORLD.Send(&a, 1, MPI_INT, (rank + 1) % n, 0);
    }
    for(int i = 0; i < 1000000; ++i);
    a = rank;
  }
  MPI::Finalize();
  return 0;
}