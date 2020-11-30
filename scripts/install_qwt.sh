#!/bin/bash

QWT_VERSION="6.1.5"
QWT_NAME="qwt-${QWT_VERSION}"

pushd "${TRAVIS_BUILD_DIR}/.."

# check if pre-built qwt was loaded from cache,
# download and build if it was not
# go into qwt dir to prepare install anyways
if find "${QWT_NAME}" -mindepth 1 | read; then
  pushd "${QWT_NAME}"
else 
  # download qwt
  wget -c -nv "https://sourceforge.net/projects/qwt/files/qwt/${QWT_VERSION}/${QWT_NAME}.zip"
  unzip -q -o "${QWT_NAME}.zip"
  pushd "${QWT_NAME}"

  # install qwt
  qmake
  make -j$(nproc)
fi

# install qwt
sudo make install

# register lib
sudo ldconfig "/usr/local/${QWT_NAME}/lib"

popd
popd
