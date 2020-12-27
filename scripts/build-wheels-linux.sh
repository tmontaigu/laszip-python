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
    "${PYBIN}/pip" wheel /io/ --no-deps -w /io/wheelhouse/
  done
}

function repair_wheel() {
  wheel="$1"
  if ! auditwheel show "$wheel"; then
    echo "Skipping non-platform wheel $wheel"
  else
    auditwheel repair "$wheel" --plat "$PLAT" -w /io/wheelhouse/
  fi
}

function repair_wheels() {
  for wheel in /io/wheelhouse/*.whl; do
    repair_wheel "$wheel"
  done
}

install_laszip 3.4.3
compile_wheels
repair_wheels
