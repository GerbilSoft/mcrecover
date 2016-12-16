#!/bin/sh
RET=0
mkdir "${TRAVIS_BUILD_DIR}/build"
cd "${TRAVIS_BUILD_DIR}/build"
cmake .. \
	-DCMAKE_INSTALL_PREFIX=/usr \
	-DCMAKE_BUILD_TYPE=Release \
	-DUSE_QT4=ON \
	-DUSE_QT5=OFF \
	|| exit 1
make || RET=1
# No tests yet...
#ctest -V || RET=1
exit "${RET}"
