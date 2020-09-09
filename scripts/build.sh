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
make INSTALL_ROOT=eNoseAnnotator -j$(nproc) install
mkdir -p eNoseAnnotator/usr/bin; cp app/app eNoseAnnotator/usr/bin
find eNoseAnnotator/


#			#
# create symbol file	#
#			#
# prepare path
crashHandlerPath="app/lib/QCrashHandler"
PATH="${PATH}:${PWD}/${crashHandlerPath}/deps/breakpad.git/src/tools/linux/dump_syms"
PATH="${PATH}:${PWD}/${crashHandlerPath}/deps/breakpad.git/src/processor"
PATH="${PATH}:${PWD}/${crashHandlerPath}/deps/breakpad.git/src/tools/linux/core2md"
PATH="${PATH}:${PWD}/${crashHandlerPath}/deps/breakpad.git/src/tools/linux/md2core"

# build breakpad tools
pushd "${PWD}/${crashHandlerPath}/deps/breakpad.git"	# cd to the breakpad dir
./configure
make
popd	# return to the root of the eNoseAnnotator repo

# strip debug info from binary
mkdir "${symbol_dir}"
objcopy --only-keep-debug "${binary_file}" "${debug_symbols_file}"
strip --strip-debug --strip-unneeded "${binary_file}"
objcopy --add-gnu-debuglink="${debug_symbols_file}" "${binary_file}"

# dump symbols
dump_syms ${debug_symbols_file} > ${symbol_file}
cp ${debug_symbols_file} ${symbol_dir}

#				#
#	generate AppImage	#
#				# 

# get linuxdeployqt
wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt-continuous-x86_64.AppImage
# export VERSION=... # linuxdeployqt uses this for naming the file

# create AppImage
prefix="eNoseAnnotator/usr"
./linuxdeployqt-continuous-x86_64.AppImage ${prefix}/share/applications/*.desktop -appimage -executable="${prefix}/lib/"

# release build: rename AppImage
if [ "$TRAVIS_PULL_REQUEST" == "false" ] && [ "$TRAVIS_TAG" != "" ]
then 
mv eNoseAnnotator*.AppImage eNoseAnnotator-${TRAVIS_TAG}-x86_64.AppImage
mv eNoseAnnotator*.AppImage.zsync eNoseAnnotator-${TRAVIS_TAG}-x86_64.AppImage.zsync
fi


# create debug symbol archive
zip -r "${symbol_dir}.zip" "${symbol_dir}"

