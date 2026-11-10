@echo off
:: SPDX-FileCopyrightText: 2026 Wunder Wungiel
:: SPDX-License-Identifier: GPL-3.0-or-later
:: Builds Wunderland for Symbian Anna/Belle (Qt 4.7.4 SDK, GCCE) and produces an unsigned .sis.
:: In-source build: Qt 4.7 + sbsv2 does not support shadow builds.
:: Usage: build_symbian.bat [release|debug]   (default: release)
::
:: Wunderland.pro deploys C:\Wunderland.png and C:\HSWidgetxWunderland_E9320736_rel.dll
:: (the prebuilt homescreen widget), so both must exist at the drive root.
setlocal
set PROJDIR=%~dp0
set CONF=%1
if "%CONF%"=="" set CONF=release

call C:\QtSDK\Symbian\SDKs\Symbian3Qt474\env.bat
pushd "%PROJDIR%"

qmake.exe Wunderland.pro -r -spec symbian-sbsv2 "CONFIG+=%CONF%" || goto :fail
make %CONF%-gcce -w || goto :fail
:: -o: create only the unsigned package (this also skips the automatic
:: capability patching).
if exist Wunderland.sis del Wunderland.sis
make sis QT_SIS_TARGET=%CONF%-gcce QT_SIS_OPTIONS=-o || goto :fail
:: createpackage names it Wunderland_unsigned.sis; keep the usual name.
move /y Wunderland_unsigned.sis Wunderland.sis >nul || goto :fail

echo.
echo === Build OK ===
dir /b *.sis
popd
exit /b 0

:fail
echo.
echo === Build FAILED ===
popd
exit /b 1
