#!/bin/bash

# find eNoseAnnotator -executable -type f -exec ldd {} \; | grep " => /usr" | cut -d " " -f 2-3 | sort | uniq # for debugging
# curl --upload-file eNoseAnnotator*.AppImage https://transfer.sh/eNoseAnnotator-git.$(git rev-parse --short HEAD)-x86_64.AppImage
if [ "$TRAVIS_PULL_REQUEST" == "false" ]
then wget -c https://github.com/probonopd/uploadtool/raw/master/upload.sh

# upload continuous branch release
if [ "$TRAVIS_TAG" == "" ]
then 
export UPLOADTOOL_SUFFIX="${TRAVIS_BRANCH}" 
fi

# upload release
if [ "$TRAVIS_TAG" != "" ]
then 
export UPLOADTOOL_SUFFIX="${TRAVIS_TAG}"
fi

bash upload.sh eNoseAnnotator*.AppImage symbols_*_linux.zip

fi
