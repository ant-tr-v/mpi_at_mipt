#include <iostream>
#include <mpi/mpi.h>
#include <random>
#include <chrono>
#include <algorithm>

template <class T>
void merge_sort(T* begin, T* end){
  ///better swith to itterators but i dont know how
  if(end - begin <= 1)
    return;
  auto len = (end - begin);
  merge_sort(begin, begin + len/2);
  merge_sort(begin + len/2, end);
  std::vector<T> tmp(len);
  std::merge(begin, begin + len/2, begin + len/2, end, tmp.begin());
  std::copy(tmp.begin(), tmp.end(), begin);
}

int main(int argc, char *argv[]) {
  MPI::Init(argc, argv);
  auto p = MPI::COMM_WORLD.Get_size(), rank = MPI::COMM_WORLD.Get_rank();
  if (argc <= 1) {
    if(rank == 0)
      std::cerr << "Not enough input data";
    MPI::Finalize();
    return -1;
  }
  int N;
  try {
    N = std::stoi(argv[1]);
  }catch(...){
    std::cerr << "Input error";
    MPI::Finalize();
    return -1;
  }
  int upper_n = (N + p - 1)/p;
  int upper_N = upper_n * p;
  std::vector<int> v(static_cast<unsigned long>(upper_N), std::numeric_limits<int>::max());
  std::vector<int> array(static_cast<unsigned long>(upper_n));
  if(rank == 0) {

    std::mt19937 gen(static_cast<unsigned long>(time(nullptr)));

    std::uniform_int_distribution<int> distribution(//-10, 10);
        std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    //generating vectors
    for (auto i = 0; i < N; ++i) {
      v[i] = distribution(gen);
    }
  }
  auto time = std::chrono::high_resolution_clock::now();
  MPI::COMM_WORLD.Scatter(v.data(), upper_n, MPI_INT, array.data(), upper_n, MPI_INT, 0);
  merge_sort(array.data(), array.data() + upper_n);
  MPI::COMM_WORLD.Gather(array.data(), upper_n, MPI_INT, v.data(), upper_n, MPI_INT, 0);
  if(rank == 0) {
    v.resize(static_cast<unsigned long>(N));
    for (int j = 2; j <= p; j *= 2) {
      for (int i = 0; i < p; i += j) {
        if (i + j / 2 < p) {
          auto end = std::min(upper_n * (i + j), upper_N);
          std::vector<int> tmp(static_cast<unsigned long>(end - upper_n * i));
          std::merge(v.data() + upper_n * i, v.data() + upper_n * (i + j / 2),
                     v.data() + upper_n * (i + j / 2), v.data() + end,
                     tmp.begin());
          std::copy(tmp.begin(), tmp.end(), v.data() + upper_n * i);
        }
      }
    }
    std::cout <<  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - time).count() <<"ms" <<std::endl;
    /// output depressed
    /*for(auto val:v)
      std::cout << val <<" ";
    std::cout<<std::endl;*/
  }
  MPI::Finalize();
  return 0;
}