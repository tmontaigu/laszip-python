set -e

function install_laszip() {
  version="$1"
  git clone -b "$version" https://github.com/laszip/laszip
  mkdir laszip-build
  cmake -B laszip-build -DCMAKE_BUILD_TYPE=Release laszip
  cmake --build laszip-build
  cmake --install laszip-build
}

function compile_wheels() {
  for PYBIN in /opt/python/*/bin; do
    is_greater_or_eq_than_3_7=$("${PYBIN}/python" -c 'import sys;v = sys.version_info;print(v.major == 3 and v.minor >= 7)')
    if [[ "$is_greater_or_eq_than_3_7" == "True" ]]; then
        "${PYBIN}/pip" wheel --no-deps . -w wheelhouse
    fi
  done
}

function repair_wheel() {
  wheel="$1"
  if ! auditwheel show "$wheel"; then
    echo "Skipping non-platform wheel $wheel"
  else
    auditwheel repair "$wheel" --plat "$PLAT" -w wheelhouse
  fi
}

function repair_wheels() {
  for wheel in wheelhouse/*.whl; do
    repair_wheel "$wheel"
  done
}

install_laszip 3.4.3

cd /laszip-python
compile_wheels
repair_wheels
