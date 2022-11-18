# laszip-python

Unofficial bindings between [Python][python-site] and [LASzip][laszip-github] made
using [pybind11][pybind11-github].

The main purpose is for integration within [pylas][pylas-github].

# Building

Building can be done using cmake or python's setup.py (which will just call cmake).

First, `laszip` and `pybind11` needs to be installed. You can install it via `vcpkg` or `conda` or other means.

To help cmake find Laszip you may have to use `-DCMAKE_TOOLCHAIN_FILE=/some/path/vcpg.cmake`
if you used vcpkg to install laszip, or `-DCMAKE_INCLUDE_PATH=...` and `-DCMAKE_LIBRARY_PATH=...`


As setup.py calls cmake the same options make need to be given:
```shell
python setup.py develop -DCMAKE_TOOLCHAIN_FILE="vcpkg.cmake"
```

[laszip-github]: https://github.com/LASzip/LASzip
[python-site]: https://www.python.org/ 
[pybind11-github]: https://github.com/pybind/pybind11
[pylas-github]: https://github.com/tmontaigu/pylas