TEMPLATE = subdirs

SUBDIRS += \
    app \

linux-g++{
    CI = $$(CI)
    # local build
    isEmpty(CI) {
        presets.files += $$PWD/usr/share/presets/*
        presets.path = $$OUT_PWD/share/presets

        INSTALLS += presets
    }
    # Travis CI build
    else {
        #isEmpty(PREFIX) {
        #    PREFIX = usr
        #}
        #target.files = app/eNoseAnnotator/usr/bin/app
        #target.path = $$PREFIX/bin

        #desktop.path = $$PREFIX/share/applications/
        #desktop.files += app/usr/share/eNoseAnnotator.desktop
        #icon512.path = $$PREFIX/share/icons/hicolor/256x256
        #icon512.files += app/usr/share/eNoseAnnotator.png

        #presets.files += app/usr/share/presets/*
        #presets.path = $$PREFIX/share/presets

        #INSTALLS += presets
        #INSTALLS += icon512
        #INSTALLS += desktop
        #INSTALLS += target

        isEmpty(PREFIX) {
            PREFIX = usr
        }

        #bin.files += app/*
        #bin.path = $$PREFIX/bin/

        presets.files += $$files("$$PWD/usr/share/presets/*", false)
        presets.path = $$PREFIX/share/presets/

        icon.files += $$PWD/usr/share/icon.png
        icon.path = $$PREFIX/share/icons/hicolor/256x256

        lib.files += $$files("$$PWD/app/lib/libtorch/*.so", true)
        lib.files += $$files("$$PWD/app/lib/libtorch/*.so.1", true)

        lib.path = $$PREFIX/lib/

        desktop.files += $$PWD/usr/share/eNoseAnnotator.desktop
        desktop.path = $$PREFIX/share/applications/

        INSTALLS += \
            presets \
            icon \
            lib \
            desktop \
    }
}
