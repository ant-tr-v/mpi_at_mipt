cmake_minimum_required(VERSION 3.8)
project(mpi)

find_package(MPI)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES hello.cpp)
add_executable(hello ${SOURCE_FILES})
add_executable(harmonic harmonic.cpp)
add_executable(round round.cpp)
add_executable(timer timer.cpp)
add_executable(exp exp.cpp MyDouble.cpp MyDouble.hpp)
add_executable(diff_eq diff-eq.cpp)