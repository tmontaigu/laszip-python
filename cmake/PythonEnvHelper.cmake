macro(ensure_pybind11_cmake_module_is_in_path)
  # Get pybind11 major version
  execute_process(
    COMMAND "${PYTHON_EXECUTABLE}" -m pybind11 --version
    RESULT_VARIABLE _pyb_res
    OUTPUT_VARIABLE _pyb_out
    ERROR_VARIABLE _pyb_err
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
  )

  if(NOT _pyb_res EQUAL 0)
    message(FATAL_ERROR
      "Failed to run '${PYTHON_EXECUTABLE} -m pybind11 --version' (exit ${_pyb_res}). "
      "Stderr: ${_pyb_err}"
    )
  endif()

  set(_ver_str "${_pyb_out}")
  # Try to capture major.minor.patch or major.minor or just major
  set(_parsed FALSE)
  string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" _m3 "${_ver_str}")
  if(NOT "${_m3}" STREQUAL "")
    set(_major "${CMAKE_MATCH_1}")
    set(_minor "${CMAKE_MATCH_2}")
    set(_patch "${CMAKE_MATCH_3}")
    set(_parsed TRUE)
  endif()

  if(NOT _parsed)
    string(REGEX MATCH "([0-9]+)\\.([0-9]+)" _m2 "${_ver_str}")
    if(NOT "${_m2}" STREQUAL "")
      set(_major "${CMAKE_MATCH_1}")
      set(_minor "${CMAKE_MATCH_2}")
      set(_patch "0")
      set(_parsed TRUE)
    endif()
  endif()

  if(NOT _parsed)
    string(REGEX MATCH "([0-9]+)" _m1 "${_ver_str}")
    if(NOT "${_m1}" STREQUAL "")
      set(_major "${CMAKE_MATCH_1}")
      set(_minor "0")
      set(_patch "0")
      set(_parsed TRUE)
    endif()
  endif()

  if(NOT _parsed)
    message(FATAL_ERROR "Could not parse pybind11 version from: '${_ver_str}'")
  endif()

  if (_major STREQUAL "3")
    set(DIR_ARG "--cmakedir")
  else()
    set(DIR_ARG "--cmake")
  endif()

  execute_process(
    COMMAND "${PYTHON_EXECUTABLE}" "-m" "pybind11" "${DIR_ARG}"
    RESULT_VARIABLE _PYTHON_SUCCESS
    OUTPUT_VARIABLE PYBIND11_CMAKE_MODULES_PATH
  )

  if(_PYTHON_SUCCESS MATCHES 0)
    string(REGEX REPLACE "\n" "" PYBIND11_CMAKE_MODULES_PATH
                         ${PYBIND11_CMAKE_MODULES_PATH}
    )
    list(INSERT CMAKE_PREFIX_PATH 0 "${PYBIND11_CMAKE_MODULES_PATH}")
  else()
    message(
      WARNING "Failed to get pybind11 cmake prefix path ${_PYTHON_SUCCESS}"
    )
  endif()
endmacro()

macro(set_python_executable_from_current_venv)
  if(WIN32)
    if(DEFINED ENV{CONDA_PREFIX})
      list(INSERT CMAKE_PREFIX_PATH 0 "$ENV{CONDA_PREFIX}/Library/share/cmake")
      set(PYTHON_EXECUTABLE "$ENV{CONDA_PREFIX}/python.exe")
    elseif(DEFINED ENV{VIRTUAL_ENV})
      set(PYTHON_EXECUTABLE "$ENV{VIRTUAL_ENV}/Scripts/python.exe")
    endif()
  else()
    if(DEFINED ENV{CONDA_PREFIX})
      set(PYTHON_EXECUTABLE "$ENV{CONDA_PREFIX}/bin/python")
    elseif(DEFINED ENV{VIRTUAL_ENV})
      set(PYTHON_EXECUTABLE "$ENV{VIRTUAL_ENV}/bin/python")
    else()
      set(PYTHON_EXECUTABLE "python3")
    endif()
  endif()
endmacro()
