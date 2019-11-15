TEMPLATE = subdirs

SUBDIRS += \
    app \

# for travis-ci.com:
# create appdir structure
linux-g++ {
    isEmpty(PREFIX) {
        PREFIX = usr
    }
    target.files = app/eNoseAnnotator/usr/bin/app
    target.path = $$PREFIX/bin

    desktop.path = $$PREFIX/share/applications/
    desktop.files += misc/eNoseAnnotator.desktop
    icon512.path = $$PREFIX/share/icons/hicolor/256x256
    icon512.files += misc/eNoseAnnotator.png

    INSTALLS += icon512
    INSTALLS += desktop
    INSTALLS += target
}


