# Add more folders to ship with the application, here
folder_01.source = qml/Wunderland
folder_01.target = qml
#DEPLOYMENTFOLDERS = folder_01

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

symbian:TARGET.UID3 = 0xE9320736

# Smart Installer package's UID
# This UID is from the protected range and therefore the package will
# fail to install if self-signed. By default qmake uses the unprotected
# range value if unprotected UID is defined for the application and
# 0x2002CCCF value if protected UID is given to the application
#symbian:DEPLOYMENT.installer_header = 0x2002CCCF

# Allow network access on Symbian
symbian:TARGET.CAPABILITY += NetworkServices #TrustedUI

# Qt Mobility and the Symbian Qt Components are only used on Symbian. Desktop Qt
# has neither, so the code that needs them is in platform/ behind
# #ifdef Q_OS_SYMBIAN (the desktop build compiles, but the QML needs
# com.nokia.symbian to run).
symbian {
    CONFIG += mobility
    MOBILITY += feedback systeminfo publishsubscribe

    CONFIG += qt-components
}

# Speed up launching on MeeGo/Harmattan when using applauncherd daemon
# CONFIG += qdeclarative-boostable

VERSION = 2.0.5
# A bare 2.0.5 token: quotes in DEFINES only survive the Symbian toolchain.
# main.cpp turns it into a string.
DEFINES += APP_Version=$$VERSION

SOURCES += main.cpp \
    appmanagement.cpp \
    newsmodel.cpp \
    appcatalog.cpp \
    installmanager.cpp \
    updatetracker.cpp \
    downloader.cpp \
    news.cpp \
    update.cpp \
    language.cpp \
    settings.cpp \
    symbian.cpp \
    logger.cpp \
    storeapi.cpp \
    applistparser.cpp \
    appentry.cpp \
    applistmodel.cpp \
    versioncompare.cpp \
    platform/apps.cpp \
    platform/device.cpp \
    platform/installerwatcher.cpp \
    platform/subscriber.cpp

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()

HEADERS += \
    appmanagement.h \
    newsmodel.h \
    appcatalog.h \
    installmanager.h \
    updatetracker.h \
    downloader.h \
    news.h \
    update.h \
    language.h \
    settings.h \
    symbian.h \
    logger.h \
    storeapi.h \
    applistparser.h \
    appentry.h \
    applistmodel.h \
    versioncompare.h \
    platform/apps.h \
    platform/device.h \
    platform/installerwatcher.h \
    platform/subscriber.h

# The homescreen widget (HSWidgetxWunderland*.dll) only exists on Symbian
symbian {
    SOURCES += platform/hswidget.cpp
    HEADERS += \
        platform/hswidget.h \
        platform/qhswidget.h \
        platform/sisregistrysession.h \
        platform/sisregistryentry.h
}

symbian {
    addFiles.pkg_postrules += "\"C:\\Wunderland.png\" - \"c:\\system\Wunderland.png\""
    addFiles.pkg_postrules += "\"C:\\HSWidgetxWunderland_E9320736_rel.dll\" - \"!:\\sys\\bin\\HSWidgetxWunderland_E9320736_rel.dll\""
    DEPLOYMENT += addFiles
}

# For lupdate
evil_hack_to_fool_lupdate {
    SOURCES += qml/Wunderland/*.qml
}

TRANSLATIONS = wunderland_template.ts \
    wunderland_de.ts \
    wunderland_el.ts \
    wunderland_es.ts \
    wunderland_fr.ts \
    wunderland_hu.ts \
    wunderland_it.ts \
    wunderland_pl.ts \
    wunderland_pt.ts \
    wunderland_ru.ts \
    wunderland_tr.ts \
    wunderland_uk.ts \
    wunderland_vi.ts

#symbian:MMP_RULES += "DEBUGGABLE"

symbian {
    DEFINES += APP_UID=\"$$TARGET.UID3\"

    ICON = icon.svg

    supported_platforms = \
      "[0x20022E6D],0,0,0,{\"S60ProductID\"}" \ # Symbian Anna
      "[0x2003A678],0,0,0,{\"S60ProductID\"}" # only Belle
    # remove default platforms
    default_deployment.pkg_prerules -= pkg_platform_dependencies
    # add our platforms
    supported_platforms_deployment.pkg_prerules += supported_platforms
    DEPLOYMENT += supported_platforms_deployment

    vendorinfo = "%{\"Wunder Wungiel\"}" ":\"Wunder Wungiel\""
    vendor_deploy.pkg_prerules += vendorinfo
    DEPLOYMENT += vendor_deploy

    LIBS += -lsisregistryclient

    #that library should be copied to SDK folder
    #LIBS += -lhswidgetpublisher
    #Libs for appswitcher
    LIBS += -lapgrfx -lcone -lws32 -lapparc -leuser
    #Libs for CFbsBimap
    LIBS += -lbitgdi -lfbscli -laknskins -laknskinsrv -leikcore
    #Libs for openDefaultBrowser
    LIBS += -lapmime
}

QT += network

RESOURCES += \
    Qml.qrc
