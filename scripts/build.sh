#!/bin/bash

#			#
#	variables	#
#			#
# file paths
binary_file="app/app"
debug_symbols_file="app/app.so"

symbol_dir="symbols_${TRAVIS_COMMIT}_linux"
symbol_file="${symbol_dir}/${symbol_dir}.sym"

#			#
# 	build app	#
#			#
qmake CONFIG+=release PREFIX=/usr
make -j$(nproc)
make INSTALL_ROOT=eNoseAnnotator -j$(nproc) install ; find eNoseAnnotator/


#			#
# create symbol file	#
#			#
# prepare path
crashHandlerPath="eNoseAnnotator/app/lib/QCrashHandler"
PATH="${PATH}:${PWD}/${crashHandlerPath}/deps/breakpad.git/src/tools/linux/dump_syms"
PATH="${PATH}:${PWD}/${crashHandlerPath}/deps/breakpad.git/src/processor"
PATH="${PATH}:${PWD}/${crashHandlerPath}/deps/breakpad.git/src/tools/linux/core2md"
PATH="${PATH}:${PWD}/${crashHandlerPath}/deps/breakpad.git/src/tools/linux/md2core"

# strip debug info from binary
mkdir "${symbol_dir}"
objcopy --only-keep-debug "${binary_file}" "${debug_symbols_file}"
strip --strip-debug --strip-unneeded "${binary_file}"
objcopy --add-gnu-debuglink="${debug_symbols_file}" "${binary_file}"

# dump symbols
dump_syms ${debug_symbols_file} > linux.sym

#				#
#	generate AppImage	#
#				# 
# prepare executable generation
# directory structure
mkdir -p eNoseAnnotator/usr/bin; mkdir -p eNoseAnnotator/share/icons/hicolor/256x256;mkdir -p eNoseAnnotator/usr/lib/;
# copy app & icon
cp app/app eNoseAnnotator/usr/bin/app; cp misc/icon.png eNoseAnnotator/share/icons/hicolor/256x256/icon.png;
# copy libtorch shared objects
cp /home/travis/build/Tilagiho/eNoseAnnotator/app/lib/libtorch/lib/*.so* eNoseAnnotator/usr/lib/
 
# get linuxdeployqt
wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt-continuous-x86_64.AppImage
# export VERSION=... # linuxdeployqt uses this for naming the file

# create AppImage       
./linuxdeployqt-continuous-x86_64.AppImage eNoseAnnotator/usr/share/applications/*.desktop -appimage -executable="eNoseAnnotator/usr/lib/"         

#				#
#	create archives		#
#				#
# create application archive
if [ "$TRAVIS_TAG" == "" ]
# no tag
# -> continuous pre-release structure
then
mkdir eNoseAnnotator-continuous-linux
mkdir eNoseAnnotator-continuous-linux/data eNoseAnnotator-continuous-linux/export eNoseAnnotator-continuous-linux/presets eNoseAnnotator-continuous-linux/classifiers 
cp eNoseAnnotator*.AppImage eNoseAnnotator-continuous-linux/
cp /home/travis/build/Tilagiho/eNoseAnnotator/misc/presets/* eNoseAnnotator-continuous-linux/presets
zip -r eNoseAnnotator-continuous-linux.zip eNoseAnnotator-continuous-linux/*
# tag -> release structure
else
DIR="eNoseAnnotator-${TRAVIS_TAG}-linux"
mkdir -p "${DIR}/data" "${DIR}/export" "${DIR}/presets" "${DIR}/classifiers"
cp eNoseAnnotator*.AppImage "${DIR}/eNoseAnnotator-${TRAVIS_TAG}.AppImage"
cp /home/travis/build/Tilagiho/eNoseAnnotator/misc/presets/* "${DIR}/presets"
zip -r "eNoseAnnotator-${TRAVIS_TAG}-linux.zip" ${DIR}/*
fi

# create debug symbol archive
zip -r "${symbol_dir}.zip" "${symbol_dir}"

