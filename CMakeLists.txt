cmake_minimum_required(VERSION 3.8)
project(mpi)

find_package(MPI)

include_directories(SYSTEM ${MPI_INCLUDE_PATH})

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES hello.cpp)
add_executable(hello ${SOURCE_FILES})
add_executable(harmonic harmonic.cpp)
add_executable(round round.cpp)
add_executable(exp exp.cpp MyDouble.cpp MyDouble.hpp)
target_link_libraries(hello ${MPI_CXX_LIBRARIES})
target_link_libraries(harmonic ${MPI_CXX_LIBRARIES})
target_link_libraries(round ${MPI_CXX_LIBRARIES})
target_link_libraries(exp ${MPI_CXX_LIBRARIES})