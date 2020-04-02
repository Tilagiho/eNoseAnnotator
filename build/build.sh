#!/bin/bash

# build app
qmake CONFIG+=release PREFIX=/usr
make -j$(nproc)
make INSTALL_ROOT=eNoseAnnotator -j$(nproc) install ; find eNoseAnnotator/
      
# prepare executable deployment:
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
    
# create zip archive
mkdir eNoseAnnotator-continuous-linux
mkdir eNoseAnnotator-continuous-linux/data eNoseAnnotator-continuous-linux/export eNoseAnnotator-continuous-linux/presets eNoseAnnotator-continuous-linux/classifiers 
cp eNoseAnnotator*.AppImage eNoseAnnotator-continuous-linux/
cp /home/travis/build/Tilagiho/eNoseAnnotator/misc/presets/* eNoseAnnotator-continuous-linux/presets
zip -r eNoseAnnotator-continuous-linux.zip eNoseAnnotator-continuous-linux/*     
