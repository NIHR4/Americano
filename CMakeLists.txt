cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(AndreHookingLibrary VERSION 0.1.0)


#add_library(AndreHookingLibrary SHARED main.cpp)
add_executable(AndreHookingLibrary main.cpp)

find_package(minhook CONFIG REQUIRED)
target_link_libraries(AndreHookingLibrary PRIVATE minhook::minhook)

