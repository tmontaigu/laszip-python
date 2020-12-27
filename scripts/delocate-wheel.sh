#!/bin/sh
set -e

# Taken from
# https://github.com/matthew-brett/delocate/issues/72#issuecomment-623070388

package_name="laszip"

origindir=$(pwd)
whl_path="$origindir/$(ls dist/*.whl | head -1)"
echo "The wheel is $whl_path"
delocate-listdeps --depending "$whl_path"

cd $(mktemp -d)
unzip "$whl_path"
delocate-path -L "$package_name".dylibs .
wheel=$(basename "$whl_path")
zip -r "$wheel" *
mv "$wheel" "$whl_path"
tempdir=$(pwd)

cd "$origindir"

rm -rf "$tempdir"

delocate-listdeps --depending "$whl_path"
