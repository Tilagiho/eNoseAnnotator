#!/bin/bash

QWT_VERSION="6.1.5"
QWT_NAME="qwt-${QWT_VERSION}"

# check if qwt was loaded from cache,
# install if it was not
if [ ! -d "../${QWT_NAME}" ]
  then
    # download qwt
    pushd ..
    wget -c -nv "https://sourceforge.net/projects/qwt/files/qwt/${QWT_VERSION}/${QWT_NAME}.zip"
    unzip -q -o "${QWT_NAME}.zip"
    pushd "${QWT_NAME}"

    # install qwt
    qmake
    make -j$(nproc)
    sudo make install
    popd
    popd
fi
# register lib
sudo ldconfig "/usr/local/${QWT_NAME}/lib"
