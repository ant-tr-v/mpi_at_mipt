#include <iostream>
#include <mpi/mpi.h>
#include <chrono>
#include "MyDouble.hpp"
#define PREC 134
using namespace std;

template<uint64_t prec>
MyDouble<prec> fibonacci(int n){
  MyDouble<prec> a(1), b(1), t;
  for(int i = 2; i < n; ++ i) {
    t = a;
    a = a + b;
    b = t;
  }
  return a;
}

template<uint64_t prec>
MyDouble<prec> fact(uint n){
  MyDouble<prec> r;
  for(uint i = 1; i <= n; ++i){
    r *= MyDouble<prec>(i);
  }
  return r;
}

template<uint64_t prec>
void count(long long int from, long long int num, MyDouble<prec> x, MyDouble<prec> *res, MyDouble<prec> *final){
  if(from == 0) {
    *res = MyDouble<prec>(1);
    *final = MyDouble<prec>(1);
  }else{
    *res = *final = x / MyDouble<prec>(from);
  }
  for(long long i = 1; i < num; ++i){
    *final *= x / MyDouble<prec>(from + i);
    *res += *final;
  }
}

int main(int argc, char *argv[]) {
  MPI::Init(argc, argv);

  auto time = std::chrono::high_resolution_clock::now();
  if (argc <= 1) {
    std::cerr << "No final number was found";
    MPI::Finalize();
    return -1;
  }
  int N = 0;
  double x =  0;
  int p = MPI::COMM_WORLD.Get_size();
  int rank = MPI::COMM_WORLD.Get_rank();
  try {
    x = std::stod(argv[1]);
    N = std::stoi(argv[2]);
  }catch(...){
    std::cerr << "Final number should be positive integer";
    MPI::Finalize();
    return -1;
  }
  bool rev = (x < 0);
  x = std::abs(x);

  long long k = N / p + static_cast<long long>(rank < N % p);

  MyDouble<PREC> res, final;
  //We represent exponent as e = 1 + x + ... x^n/n! * ( x/(n + 1) + ... x^(m - n)*n!/m! * ( ...))
  count<PREC>(rank*(N / p) + std::min(rank, N % p), k, MyDouble<PREC>(x), &res, &final);

  int size = static_cast<int>(res.size());
  int32_t exp;

  if(rank != p - 1){
    auto *data = new uint32_t[size];
    MPI::COMM_WORLD.Recv(&exp, 1, MPI_INT32_T, MPI_ANY_SOURCE, MPI_ANY_TAG);
    MPI::COMM_WORLD.Recv(data, size, MPI_INT32_T, MPI_ANY_SOURCE, MPI_ANY_TAG);
    MyDouble<PREC> ret(exp, static_cast<size_t>(size), data);
    res += final * ret;
    delete[] data;
  }
  if(rank){
    exp = res.exponent();
    MPI::COMM_WORLD.Send(&exp, 1, MPI_INT32_T, (rank - 1), 0);
    MPI::COMM_WORLD.Send(res.data(), size, MPI_UINT32_T, (rank - 1), 0);
  } else {
    if(rev)
      res = MyDouble<PREC>(1)/res;
    long long t1 = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - time).count();
    std::cout << res.dec() << endl;
    long long t2 = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - time).count();
    std::cout << t1  <<"ms (+ " << t2 - t1 <<"ms)" << endl;
  }
  MPI::Finalize();
  return 0;
}
