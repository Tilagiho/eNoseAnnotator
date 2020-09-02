# If there is no version tag in git this one will be used
VERSION = 0.1.0

# Need to discard STDERR so get path to NULL device
win32 {
    NULL_DEVICE = NUL # Windows doesn't have /dev/null but has NUL
} else {
    NULL_DEVICE = /dev/null
}

# Need to call git with manually specified paths to repository
BASE_GIT_COMMAND = git

# get last annotated tag
GIT_VERSION = $$system($$BASE_GIT_COMMAND describe 2> $$NULL_DEVICE)

# add commit count since last annotated tag
# GIT_COMMIT_COUNT = $$system($$BASE_GIT_COMMAND rev-list $(git describe --abbrev=0)..HEAD --count 2> $$NULL_DEVICE)
# GIT_VERSION = "$${GIT_VERSION}_$${GIT_COMMIT_COUNT}"

# Now we are ready to pass parsed version to Qt
# VERSION = $$GIT_VERSION
# win32 { # On windows version can only be numerical so remove commit hash, the prefix "v" & _ in front of the commit count
#    VERSION ~= s/\.\d+\.[a-f0-9]{6,}//
#    VERSION ~= s/v//
#    VERSION ~= s/_/"."/
#    VERSION ~= s/[a-z0-9]{8}//
# }

# Adding C preprocessor #DEFINE so we can use it in C++ code
# also here we want full version on every system so using GIT_VERSION
DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"

# By default Qt only uses major and minor version for Info.plist on Mac.
# This will rewrite Info.plist with full version
macx {
    INFO_PLIST_PATH = $$shell_quote($${OUT_PWD}/$${TARGET}.app/Contents/Info.plist)
    QMAKE_POST_LINK += /usr/libexec/PlistBuddy -c \"Set :CFBundleShortVersionString $${VERSION}\" $${INFO_PLIST_PATH}
}

