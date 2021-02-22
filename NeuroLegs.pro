QT       += core gui
QT       += network charts

RC_ICONS = foot.ico

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    calibrationwindow.cpp \
    eventsfunctions.cpp \
    main.cpp \
    mainwindow.cpp \
    udpthread.cpp

HEADERS += \
    calibrationwindow.h \
    mainwindow.h

FORMS += \
    calibrationwindow.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


#PRECOMPILED_HEADER = precomp_head
#CONFIG += precompile_header

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml \
    foot.ico

RESOURCES += \
    res.qrc


#ANDROID_PACKAGE_SOURCE_DIR = $$PWD/file

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}
