# SPDX-FileCopyrightText: Sebastian (schumi1331)
# SPDX-FileCopyrightText: 2026 Wunder Wungiel
# SPDX-License-Identifier: GPL-3.0-or-later

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
# has neither, so the code that needs them is in src/platform/ behind
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

INCLUDEPATH += src

SOURCES += main.cpp \
    src/appmanagement.cpp \
    src/newsmodel.cpp \
    src/appcatalog.cpp \
    src/installmanager.cpp \
    src/updatetracker.cpp \
    src/downloader.cpp \
    src/news.cpp \
    src/update.cpp \
    src/language.cpp \
    src/settings.cpp \
    src/symbian.cpp \
    src/logger.cpp \
    src/storeapi.cpp \
    src/applistparser.cpp \
    src/appentry.cpp \
    src/applistmodel.cpp \
    src/versioncompare.cpp \
    src/platform/apps.cpp \
    src/platform/device.cpp \
    src/platform/installerwatcher.cpp \
    src/platform/subscriber.cpp

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()

HEADERS += \
    src/appmanagement.h \
    src/newsmodel.h \
    src/appcatalog.h \
    src/installmanager.h \
    src/updatetracker.h \
    src/downloader.h \
    src/news.h \
    src/update.h \
    src/language.h \
    src/settings.h \
    src/symbian.h \
    src/logger.h \
    src/storeapi.h \
    src/applistparser.h \
    src/appentry.h \
    src/applistmodel.h \
    src/versioncompare.h \
    src/platform/apps.h \
    src/platform/device.h \
    src/platform/installerwatcher.h \
    src/platform/subscriber.h

# The homescreen widget (HSWidgetxWunderland*.dll) only exists on Symbian
symbian {
    SOURCES += src/platform/hswidget.cpp
    HEADERS += \
        src/platform/hswidget.h \
        src/platform/qhswidget.h \
        src/platform/sisregistrysession.h \
        src/platform/sisregistryentry.h
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

TRANSLATIONS = translations/wunderland_template.ts \
    translations/wunderland_de.ts \
    translations/wunderland_el.ts \
    translations/wunderland_es.ts \
    translations/wunderland_fr.ts \
    translations/wunderland_hu.ts \
    translations/wunderland_it.ts \
    translations/wunderland_pl.ts \
    translations/wunderland_pt.ts \
    translations/wunderland_ru.ts \
    translations/wunderland_tr.ts \
    translations/wunderland_uk.ts \
    translations/wunderland_vi.ts

# Compile the .ts files into .qm with lrelease during the build (Qt 4 has no
# embed_translations; this is the same extra-compiler setup as the SDK's loc.prf).
# The .qm files land next to the .ts files and are embedded through Qml.qrc;
# they are not committed. The template is listed for lupdate only.
qtPrepareTool(LRELEASE, lrelease)
LRELEASE_INPUT = $$TRANSLATIONS
LRELEASE_INPUT -= translations/wunderland_template.ts
lrelease.input = LRELEASE_INPUT
lrelease.output = $$PWD/translations/${QMAKE_FILE_BASE}.qm
lrelease.commands = $$LRELEASE -silent ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
lrelease.CONFIG = no_link target_predeps
QMAKE_EXTRA_COMPILERS += lrelease
# qmake reads Qml.qrc (rcc -list) to find rcc's dependencies, so the .qm files
# must already exist then (e.g. in a fresh checkout); otherwise rcc wouldn't be
# rerun when a .ts changes.
for(ts, LRELEASE_INPUT) {
    qm = $$PWD/$$replace(ts, \\.ts$, .qm)
    !exists($$qm): system($$LRELEASE -silent $$PWD/$$ts -qm $$qm)
}

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
    Qml.qrc \
    translations.qrc
