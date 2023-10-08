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
  # -L makes find follow symlinks, which
  # is what folders we are looking python in are
  pybins=$(find -L /opt/python -regex '.*\/bin\/python3.[0-9]+$')
  for pybin in $pybins; do
    is_greater_or_eq_than_3_7=$("${pybin}" -c 'import sys;v = sys.version_info;print(v.major == 3 and v.minor >= 7)')
    echo "Considering $pybin -> >= 3.7 ? $is_greater_or_eq_than_3_7"
    if [[ "$is_greater_or_eq_than_3_7" == "True" ]]; then
        "${pybin}" -m pip wheel --no-deps . -w wheelhouse
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
    rm "$wheel" # Remove non repaired wheel
  done
}

install_laszip 3.4.3

cd /laszip-python
compile_wheels
repair_wheels
