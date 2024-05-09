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

option(USE_VENDORED_LASZIP "Build by building and linking to the vendored LASZip" ON)

if (USE_VENDORED_LASZIP)
    option(LASZIP_BUILD_STATIC "Build LASZip as static lib" ON)
    set(LASZIP_VENDOR "LASzip-3.4.4")
    add_subdirectory(src/vendor/${LASZIP_VENDOR})
else()
    find_package(LASzip MODULE REQUIRED)
endif()

pybind11_add_module(laszip
        src/laszip.cpp
        src/laszip_error.h
        src/lasunzipper.h
        src/lasunzipper.cpp
        src/laszipper.h
        src/laszipper.cpp
        src/python_istreambuf.h
        src/python_ostreambuf.h)

if (USE_VENDORED_LASZIP)
    target_compile_definitions(laszip PRIVATE USE_VENDORED_LASZIP)
    target_include_directories(laszip
        PRIVATE
        src/vendor/${LASZIP_VENDOR}/dll
    )
    target_link_libraries(laszip PRIVATE  laszip_base)
    set_target_properties(laszip
        PROPERTIES
        POSITION_INDEPENDENT_CODE TRUE
    )
else()
    target_link_libraries(laszip PRIVATE LASzip::LASzip)
endif()

install(TARGETS laszip DESTINATION laszip)
if (WIN32)
    install(FILES ${LASZIP_DLL} DESTINATION laszip)
endif()
