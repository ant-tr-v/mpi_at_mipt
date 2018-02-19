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
    MPI::COMM_WORLD.Send(&a, 1, MPI_INT, (rank + 1) % n, 0);
    MPI::COMM_WORLD.Recv(&a, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG);
    std::cout <<  "[" << rank <<"] Received message from " << a << std::endl;
    for(int i = 0; i < 1000000; ++i);
  }else{
    int a;
    MPI::COMM_WORLD.Recv(&a, 1, MPI_INT,  MPI_ANY_SOURCE, MPI_ANY_TAG);
    std::cout <<  "[" << rank <<"] Received message from " << a << std::endl;
    for(int i = 0; i < 1000000; ++i);
    a = rank;
    MPI::COMM_WORLD.Send(&a, 1, MPI_INT, (rank + 1) % n, 0);
  }
  MPI::Finalize();
  return 0;
}