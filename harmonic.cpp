#include <iostream>
#include <mpi/mpi.h>
#include <chrono>


inline long double count(long long int from, long long int num){
  long double res = 0;
  for(long long i = 0; i < num; ++i){
    //std:: cout <<from <<" " << (i + from) << std::endl;
    res += 1/ static_cast<long double>(i + from);
  }
  return res;
}

int main(int argc, char *argv[]) {
  MPI::Init(argc, argv);
  auto time = std::chrono::high_resolution_clock::now();
  if (argc <= 1) {
    std::cerr << "No final number was found";
    MPI::Finalize();
    return -1;
  }
  long long N = 0;
  long long p = MPI::COMM_WORLD.Get_size();
  long long rank = MPI::COMM_WORLD.Get_rank();
  try {
    N = std::stoll(argv[1]);
  }catch(...){
    std::cerr << "Final number should be positive integer";
    MPI::Finalize();
    return -1;
  }
  long long k = N / p + static_cast<long long>(rank < N % p);
  long double res = count(rank*(N / p) + std::min (rank, N % p) + 1, k);
  if(rank)
    MPI::COMM_WORLD.Send(&res, 1, MPI_LONG_DOUBLE, 0, 0);
  else{
    for(long long i = 1; i < p; ++i){
      long double t = 0;
      MPI::COMM_WORLD.Recv(&t, 1, MPI_LONG_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG);
      res += t;
    }
    std::cout.precision(10);
    std:: cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - time).count() << " ms\n";
    std:: cout << res << "\n";
  }
  MPI::Finalize();
  return 0;
}