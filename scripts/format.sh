#!/bin/bash

SCRIPT_DIR=$(cd $(dirname ${BASH_SOURCE[0]});pwd)
cd ${SCRIPT_DIR}/..
pwd

find_sources="find . -type f -name *\.h -o -name *\.cpp | grep -v third_party"
echo -n "Running dos2unix     "
$find_sources | xargs -I {} sh -c "dos2unix '{}' 2>/dev/null; echo -n '.'"
echo

echo -n "Running clang-format "
$find_sources | xargs -I {} sh -c "clang-format -i {}; echo -n '.'"
echo

echo -n "Running cmake-format "
find . -type f -name "CMakeLists.txt" -o -name "*\.cmake" | \
  grep -v bundled | grep -v build | grep -v third_party \
  xargs -I {} sh -c "cmake-format --line-width 120 --tab-size 4 --max-subgroups-hwrap 4 -i {}; echo -n '.'"
echo



