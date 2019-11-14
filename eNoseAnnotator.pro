TEMPLATE = subdirs

SUBDIRS += \
    app \
    test


linux-g++ {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    target.path = $$PREFIX/bin

    desktop.path = $$PREFIX/share/applications/
    desktop.files += misc/eNoseAnnotator.desktop
    icon512.path = $$PREFIX/share/icons/hicolor/512x512/apps
    icon512.files += misc/eNoseAnnotator.png

    INSTALLS += icon512
    INSTALLS += desktop
    INSTALLS += target
}
