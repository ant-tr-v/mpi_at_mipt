#include <iostream>
#include <boost/mpi.hpp>
#include <chrono>

using namespace std;

//specifying functions and constants used;
const double A = 2;
const double pi = 3.14159265358979323846;

inline double phi(double x){
  return cos(pi*x);
}

inline double psi(double t){
  return exp(-t);
}

inline double func(double x, double t){
  return x + t;
}

//analytic solution for functions above
inline double solution(double x, double t){
  if(x > 2 * t)
    return x * t - t * t / 2 + cos(pi * (2*t - x));
  return x * t - t * t / 2 + (2*t - x)*(2*t - x) / 8 + exp(-(t - x / 2));
}

double calculate_line(double *dst, const double *src, double left_value,
                      double left_down_value, double tau, double h, int col, int row, int n){
  /// tree points used tu calculate result in upper right point the last one is returned
  double ll =  left_value, ld = left_down_value, rd;
  for(int i = 0; i < n; ++i){
    rd = src[i];
    double f = func((col+ i + 0.5)*h, (row+ 0.5)*tau);
    dst[i] = (2 * f * h * tau / A + h / A * (ld + rd - ll) + tau * (ll + ld - rd)) / (tau + h / A);
    ld = rd;
    ll = dst[i];
  }
  return dst[n - 1];
}

int main(int argc, char *argv[]) {
  MPI::Init(argc, argv);
  auto time = chrono::high_resolution_clock::now();
  int p = MPI::COMM_WORLD.Get_size();
  int rank = MPI::COMM_WORLD.Get_rank();
  if (argc <= 2) {
    if(rank == 0)
      std::cerr << "Not enough input data";
    MPI::Finalize();
    return -1;
  }
  double tau;
  double h =  0;
  try {
    tau = std::abs(std::stod(argv[argc - 2]));
    h = std::abs(std::stod(argv[argc - 1]));
  }catch(...){
    std::cerr << "Input error";
    MPI::Finalize();
    return -1;
  }
  auto J = static_cast<unsigned long>(1./tau);
  auto N = static_cast<unsigned long>(1./h);
  unsigned long n = (N - 1) / p + static_cast<unsigned long>(rank < (N - 1) % p);
  vector<vector<double> >res(J);
  for(int i = 0; i < J; ++i){
    res[i].resize(N + 1, 0);
    res[i][0] = psi(i* tau);
  }
  for(int j = 0; j < N; ++j) {
    res[0][j] = phi(j * h);
  }
  unsigned long start = rank*((N - 1) / p) + std::min(rank, int((N - 1) % p));
  double ld = res[0][start];
  for(int i = 0; i < J - 1; ++i){

    double ll = res[i + 1][0];
    if(rank)
       MPI::COMM_WORLD.Recv(&ll, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG);
    auto new_ll = calculate_line(res[i + 1].data() + start + 1, res[i].data() + start + 1,
                                 ll, ld, tau, h, static_cast<int>(start + 1), i, static_cast<int>(n));
    if(rank != p - 1)
      MPI::COMM_WORLD.Send(&new_ll, 1, MPI_DOUBLE, rank + 1, 0);
    ld = ll;
  }

  if(rank == p - 1)
    std::cout <<  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - time).count() <<"ms" <<std::endl;
  //output is deprecated
  //accuracy by max deviation
  double sigm = 0, acc = 0;
  for(int i =0; i < J; ++i){
    //vector<double> line(N);
    //MPI::COMM_WORLD.Reduce(res[i].data(), line.data(), static_cast<int>(N), MPI_DOUBLE, MPI_SUM, 0);
    for (auto j = start + 1; j < start + 1 + n; ++j) {
      auto t = res[i][j];
      acc = max(acc, abs(t - solution(j*h, i*tau)));
      sigm += (t - solution(j*h, i*tau))*(t - solution(j*h, i*tau));
    }
  }
  double sigmall = 0, accall = 0;
  MPI::COMM_WORLD.Reduce(&acc, &accall, 1, MPI_DOUBLE, MPI_MAX, 0);
  MPI::COMM_WORLD.Reduce(&sigm, &sigmall, 1, MPI_DOUBLE, MPI_SUM, 0);
  if(rank == 0)
    std::cout <<  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - time).count() <<"ms" <<std::endl;
  if(rank == 0)
    cout << "accuracy:\t" << accall <<"\nsigma:\t\t" << sqrt(sigmall/(N*J));
  MPI::Finalize();
  return 0;
}