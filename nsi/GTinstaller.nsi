;NSIS Installer Script for QLandkarte GT

;NSIS References/Documentation 
;http://nsis.sourceforge.net/Docs/Modern%20UI%202/Readme.html
;http://nsis.sourceforge.net/Docs/Modern%20UI/Readme.html

;Revision Log
; 01-Jun-2010 Rework Installer - create batch file to set FWTools environment correctly
; 24-Jun-2010 Copy all transölation files
; 23-Oct-2010 Rework installation of Microsoft Runtime Libraries, pass through parameters in start script, add german translation

;=================== BEGIN SCRIPT ====================
; Include for nice Setup UI
!include MUI2.nsh

;------------------------------------------------------------------------
; Modern UI2 definition							                                    -
;------------------------------------------------------------------------
; Description
Name "QLandkarte GT"

;Default installation folder
InstallDir "$PROGRAMFILES\QLandkarteGT"

;Get installation folder from registry if available
InstallDirRegKey HKCU "Software\QLandkarteGT" ""

;Request application privileges for Windows Vista
RequestExecutionLevel admin


; The file to write
OutFile "QLandkarteGT.exe"

;------------------------------------------------------------------------
; Modern UI definition   	   					                                  -
;------------------------------------------------------------------------
;!define MUI_COMPONENTSPAGE_SMALLDESC ;No value
!define MUI_INSTFILESPAGE_COLORS "FFFFFF 000000" ;Two colors

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "logo_small.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP "logo_big.bmp"

; Page welcome description
!define MUI_WELCOMEPAGE_TITLE "QLandkarte GT"
!define MUI_WELCOMEPAGE_TITLE_3LINES
!define MUI_WELCOMEPAGE_TEXT "This is a GeoTiff viewer for the PC. Next to displaying maps it is able to visualize data acquired by a GPSr on the map. With QLandkarte GT you are able to produce smaller map subsets to be used by mobile devices."

!define MUI_LICENSEPAGE_CHECKBOX

;------------------------------------------------------------------------
; Pages definition order						                                    -
;------------------------------------------------------------------------
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "License.rtf"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
Var StartMenuFolder
!insertmacro MUI_PAGE_STARTMENU "Application" $StartMenuFolder
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
;------------------------------------------------------------------------

;------------------------------------------------------------------------
;Uninstaller                                                            -
;------------------------------------------------------------------------
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Language settings
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "German"


;------------------------------------------------------------------------
; Component add		   	   					                                      -
;------------------------------------------------------------------------
;Components description

Section "FWTools 2.4.7" FWTools

	ReadRegStr $0 HKLM "Software\FWtools" Install_Dir
	!define TMP_PATH $0

	ExecWait '"${TMP_PATH}\uninstall.exe"'

	; Don't do it if we can package install
 	NSISdl::download http://home.gdal.org/fwtools/FWTools247.exe $TEMP\FWTools247.exe
 	Pop $R0 ;Get the return value
 	StrCmp $R0 "success" +3
 	  MessageBox MB_OK "Download failed: $R0"
 	  Quit
 	ExecWait '"$TEMP\FWTools247.exe"'

SectionEnd
LangString DESC_FWTools ${LANG_ENGLISH} "Required Libraries for MAP and coordinate system transformation. This package will be downloaded from the internet. It needs to be installed only once - for the first QLandkarteGT installation."
LangString DESC_FWTools ${LANG_GERMAN}  "Benötigte Bibliotheken für Karten- und Koordinatentransformationen. Wird aus dem Internet heruntergeladen. Muss nur einmal installiert werden - bei der ersten QLandkarteGT Installation."

Section "MSVC++ 2008 SP1 Runtime" MSVC

  SetOutPath $INSTDIR
  File Files\vcredist_x86.exe
  ExecWait '"$INSTDIR\vcredist_x86.exe"'
  Delete "$INSTDIR\vcredist_x86.exe"
  
SectionEnd
LangString DESC_MSVC ${LANG_ENGLISH} "Microsoft Visual C++ 2008 SP1 Runtime Libraries. Typically already installed on your PC. You only need to install them if it doesn't work without ;-)."
LangString DESC_MSVC ${LANG_GERMAN} "Microsoft Visual C++ 2008 SP1 Laufzeitbibliotheken. Diese sind meist bereits auf dem Rechner installiert. Versuchen Sie die Installation zunächst einmal ohne dies."

Section "QLandkarte GT" QLandkarteGT

  ;Install for all users
  SetShellVarContext all

  SetOutPath $INSTDIR
    File Files\qlandkartegt.exe
    File Files\Globe128x128.ico
    File Files\qlandkartegt_*.qm
    File Files\qt_??.qm

  ;BEGIN Qt Files
	SetOutPath $INSTDIR
  	File Files\QtCore4.dll
  	File Files\QtGui4.dll
  	File Files\QtNetwork4.dll
  	File Files\QtSvg4.dll
  	File Files\QtXml4.dll
    File Files\QtOpenGL4.dll
    File Files\QtSql4.dll

	SetOutPath "$INSTDIR\imageformats\"
  	File Files\imageformats\qgif4.dll
  	File Files\imageformats\qjpeg4.dll
  	File Files\imageformats\qmng4.dll
  	File Files\imageformats\qsvg4.dll

	SetOutPath "$INSTDIR\sqldrivers\"
    File Files\sqldrivers\qsqlite4.dll
    
  ;the last "SetOutPath" will be the default directory
  SetOutPath $INSTDIR    
    
  ;END Qt Files

  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ;create batch file to run qlandkartegt.exe
  ReadRegStr $0 HKLM "Software\FWtools" Install_Dir
  StrCpy $1 "call $\"$0\setfw.bat$\"$\r$\n"
  fileOpen $0 "$INSTDIR\QLandkarteGT.bat" w
  fileWrite $0 $1
  fileWrite $0 "start qlandkartegt.exe %*"
  fileClose $0

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
   	;Create shortcuts
  	CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\QLandkarteGT.lnk" "$INSTDIR\QLandkarteGT.bat" "" "$INSTDIR\Globe128x128.ico"
 	!insertmacro MUI_STARTMENU_WRITE_END

  ;Create registry entries
	WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\QLandkarte GT" "DisplayName" "QLandkarte GT (remove only)"
	WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\QLandkarte GT" "UninstallString" "$INSTDIR\Uninstall.exe"

SectionEnd
LangString DESC_QLandkarteGT ${LANG_ENGLISH} "View GeoTiff and Garmin Maps. Visualize and analyze GPX files and much more!"
LangString DESC_QLandkarteGT ${LANG_GERMAN}  "Landkarten im GeoTiff und Garmin Format betrachten. GPX Dateien visualisieren und analysieren und vieles mehr!"


!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
   !insertmacro MUI_DESCRIPTION_TEXT ${QLandkarteGT} $(DESC_QLandkarteGT)
   !insertmacro MUI_DESCRIPTION_TEXT ${FWTools} $(DESC_FWTools)
   !insertmacro MUI_DESCRIPTION_TEXT ${MSVC} $(DESC_MSVC)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;------------------------------------------------------------------------
;Uninstaller Sections							                                      -
;------------------------------------------------------------------------
Section "Uninstall"

  ;Install for all users
  SetShellVarContext all

  Delete "$INSTDIR\Uninstall.exe"

  SetOutPath $TEMP

  RMDir /r $INSTDIR

  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder

  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\QLandkarteGT.lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"

  DeleteRegKey /ifempty HKCU "Software\QLandkarteGT"
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\QLandkarte GT"

SectionEnd


