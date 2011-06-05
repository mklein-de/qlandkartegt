rem Batch file to copy the necessary files for the Windows Installer
rem Please adapt environment variables in section 1) to your system

rem Section 1.) Define path to Qt, MSVC, .... installations
set QLGTI_QT_PATH=C:\Qt\4.7.0
set QLGTI_VCREDIST_PATH="E:\qlgt\tools\vcredist_VS2008SP1"
set QLGTI_LIBEXIF_PATH="E:\qlgt\tools\libexif"

rem Section 2.) Copy Files
del /s/q Files
mkdir Files
cd Files
rem Section 2.1) Copy Qt files
copy %QLGTI_QT_PATH%\bin\QtCore4.dll
copy %QLGTI_QT_PATH%\bin\QtGui4.dll
copy %QLGTI_QT_PATH%\bin\QtNetwork4.dll
copy %QLGTI_QT_PATH%\bin\QtOpenGL4.dll
copy %QLGTI_QT_PATH%\bin\QtSql4.dll
copy %QLGTI_QT_PATH%\bin\QtSvg4.dll
copy %QLGTI_QT_PATH%\bin\QtWebKit4.dll
copy %QLGTI_QT_PATH%\bin\phonon4.dll
copy %QLGTI_QT_PATH%\bin\QtXml4.dll
mkdir imageformats
cd imageformats
copy %QLGTI_QT_PATH%\plugins\imageformats\qgif4.dll
copy %QLGTI_QT_PATH%\plugins\imageformats\qjpeg4.dll
copy %QLGTI_QT_PATH%\plugins\imageformats\qmng4.dll
copy %QLGTI_QT_PATH%\plugins\imageformats\qsvg4.dll
cd ..
mkdir sqldrivers
cd sqldrivers
copy %QLGTI_QT_PATH%\plugins\sqldrivers\qsqlite4.dll
cd ..
rem  The qt_??.qm files must have been created before by
rem opening a qt shell, going to the translations directory and running
rem for %f in (qt_??.ts) do lrelease %f
copy %QLGTI_QT_PATH%\translations\qt_??.qm
rem section 2.2) Copy MSVC Redist Files
copy %QLGTI_VCREDIST_PATH%\vcredist_x86.exe
rem section 2.3) Copy libexif Files
copy %QLGTI_LIBEXIF_PATH%\libexif-12.dll
rem section 2.4) Copy QLandkarte GT Files
copy ..\..\build\bin\Release\qlandkartegt.exe
copy ..\..\build\bin\Release\map2gcm.exe
copy ..\..\build\src\*.qm
copy ..\..\src\icons\Globe128x128.ico
rem section 2.5) 3rd party SW description
copy ..\3rdparty.txt

pause