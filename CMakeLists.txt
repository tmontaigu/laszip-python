cmake_minimum_required(VERSION 3.17)
project(laszipy)

set(CMAKE_CXX_STANDARD 14)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(pybind11)
find_package(LASzip REQUIRED)

pybind11_add_module(laszipy
        src/laszipy.cpp
        src/laszip_error.h
        src/lasunzipper.h
        src/lasunzipper.cpp
        src/laszipper.h
        src/laszipper.cpp
        src/python_istreambuf.h
        src/python_ostreambuf.h)

target_link_libraries(laszipy PRIVATE ${LASZIP_LIBRARIES})
target_include_directories(laszipy PRIVATE ${LASZIP_INCLUDE_DIR})

