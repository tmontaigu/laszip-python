cmake_minimum_required(VERSION 3.16)
project(laszip-python)

set(CMAKE_CXX_STANDARD 14)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if(NOT SKBUILD)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PythonEnvHelper.cmake)
    set_python_executable_from_current_venv()
    ensure_pybind11_cmake_module_is_in_path()
endif()

find_package(pybind11)
find_package(LASzip MODULE REQUIRED)

pybind11_add_module(laszip
        src/laszip.cpp
        src/laszip_error.h
        src/lasunzipper.h
        src/lasunzipper.cpp
        src/laszipper.h
        src/laszipper.cpp
        src/python_istreambuf.h
        src/python_ostreambuf.h)

target_link_libraries(laszip PRIVATE LASzip::LASzip)

install(TARGETS laszip DESTINATION laszip)
if (WIN32)
    install(FILES ${LASZIP_DLL} DESTINATION laszip)
endif()
