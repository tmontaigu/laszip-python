cmake_minimum_required(VERSION 3.16)
project(laszip-python)

set(CMAKE_CXX_STANDARD 14)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(pybind11)
find_package(LASzip REQUIRED)

pybind11_add_module(laszip
        src/laszip.cpp
        src/laszip_error.h
        src/lasunzipper.h
        src/lasunzipper.cpp
        src/laszipper.h
        src/laszipper.cpp
        src/python_istreambuf.h
        src/python_ostreambuf.h)

target_link_libraries(laszip PRIVATE ${LASZIP_LIBRARIES})
target_include_directories(laszip PRIVATE ${LASZIP_INCLUDE_DIR})

message(${LASZIP_LIBRARY})

if (NOT WIN32)
    option(COPY_LASZIP_LIB "Shall the the laszip lib be copied next to the python module" OFF)
    if (COPY_LASZIP_LIB)
        add_custom_command(
                TARGET laszip
                POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${LASZIP_LIBRARY} $<TARGET_FILE_DIR:laszip>)
    endif()
endif()

