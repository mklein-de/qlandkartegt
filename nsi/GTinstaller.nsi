; Include for nice Setup UI
!include MUI2.nsh
!include WriteEnvStr.nsh

; Variable declaration
!define PATH "%FWTOOLS_DIR%\bin;%FWTOOLS_DIR%\python;%PATH%"
!define PYTHONPATH "%FWTOOLS_DIR%\pymod"
!define PROJ_LIB "%FWTOOLS_DIR%\proj_lib"
!define GEOTIFF_CSV "%FWTOOLS_DIR%\data"
!define GDAL_DATA "%FWTOOLS_DIR%\data"
!define GDAL_DRIVER_PATH "%FWTOOLS_DIR%\gdal_plugins"


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
  RequestExecutionLevel user  

  ReserveFile "${NSISDIR}\Plugins\InstallOptions.dll"
  ReserveFile "GTInstaller.ini"

;------------------------------------------------------------------------
; Moder UI definition   	   					                                  -
;------------------------------------------------------------------------
!define MUI_COMPONENTSPAGE_SMALLDESC ;No value
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

Function .onInit

  ;Extract InstallOptions files
  ;$PLUGINSDIR will automatically be removed when the installer closes
  
  InitPluginsDir
  File /oname=GTInstaller.ini "GTInstaller.ini"
  
FunctionEnd

;------------------------------------------------------------------------
; Component add		   	   					                                      -
;------------------------------------------------------------------------
;Components description

  Section "FWTools" FWTools
  	SetOutPath $INSTDIR
  	; Don't do it if we can package install
  	NSISdl::download http://home.gdal.org/fwtools/FWTools210.exe $TEMP\FWTools210.exe
  	Pop $R0 ;Get the return value
  	  StrCmp $R0 "success" +3
  	    MessageBox MB_OK "Download failed: $R0"
  	    Quit
  	ExecWait '"$TEMP\FWTools210.exe"'    
  SectionEnd
  LangString DESC_FWTools ${LANG_ENGLISH} "FWTools includes OpenEV, GDAL, MapServer, PROJ.4 and OGDI as well as some supporting components."


  Section "QLandkarteGT" QLandkarteGT
  	SetOutPath $INSTDIR
  	File Files\qlandkartegt.exe
  	WriteUninstaller "$INSTDIR\Uninstall.exe"
  	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
      	;Create shortcuts
  	    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
  	    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  	    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\QLandkarteGT.lnk" "$INSTDIR\QLandkarteGT.exe"  
  	!insertmacro MUI_STARTMENU_WRITE_END
	
	WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\QLandkarte GT" "DisplayName" "QLandkarte GT (remove only)"
	WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\QLandkarte GT" "UninstallString" "$INSTDIR\Uninstall.exe"
	
    !define ReadEnvStr_RegKey 'HKCU "Environment"'
    ; Set environment variable

    ReadRegStr $0 HKLM "Software\FWtools" Install_Dir
    StrCmp $0 "" 0 +6
      ReadRegStr $0 HKCU "Software\FWtools" Install_Dir
      StrCmp $0 "" 0 +4
      InstallOptions::dialog "GTInstaller.ini"
      ReadINIStr $2 "GTInstaller.ini" "FWTools" "State"
      StrCpy $0 $2    
    !define FWTOOLS_DIR $0
    Push FWTOOLS_DIR
    Push '${FWTOOLS_DIR}'
    Call WriteEnvStr
    ReadEnvStr $1 "PATH"
    StrCpy $1 "$1;$0\bin;$0\python"
    Push PATH
    Push '${PATH}'
    Call WriteEnvStr
    Push PYTHONPATH
    Push '${PYTHONPATH}'
    Call WriteEnvStr
    Push PROJ_LIB
    Push '${PROJ_LIB}'
    Call WriteEnvStr
    Push GEOTIFF_CSV
    Push '${GEOTIFF_CSV}'
    Call WriteEnvStr
    Push GDAL_DATA
    Push '${GDAL_DATA}'
    Call WriteEnvStr
    Push GDAL_DRIVER_PATH
    Push '${GDAL_DRIVER_PATH}'
    Call WriteEnvStr
	
  SectionEnd
  LangString DESC_QLandkarteGT ${LANG_ENGLISH} "This is a GeoTiff viewer for the PC"
    
  Section "QT 4.3" QT
  	File Files\qgif4.dll
  	File Files\qjpeg4.dll
  	File Files\qmng4.dll
  	File Files\qsvg4.dll
  	File Files\QtCore4.dll
  	File Files\QtGui4.dll
  	File Files\QtNetwork4.dll
  	File Files\QtSvg4.dll
  	File Files\QtXml4.dll
  	SetOutPath $INSTDIR
  SectionEnd
  LangString DESC_QT ${LANG_ENGLISH} "QT required dependencies."

  Section "MSVC 8.0" MSVC
   	;File Files\msvcm80.dll
  	;File Files\msvcp80.dll
  	;File Files\msvcr80.dll
  	SetOutPath $INSTDIR
  SectionEnd
  LangString DESC_MSVC ${LANG_ENGLISH} "Microsoft Visual C Runtime Libraries."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
   !insertmacro MUI_DESCRIPTION_TEXT ${QLandkarteGT} $(DESC_QLandkarteGT)
   !insertmacro MUI_DESCRIPTION_TEXT ${FWTools} $(DESC_FWTools)
   !insertmacro MUI_DESCRIPTION_TEXT ${QT} $(DESC_QT)
   !insertmacro MUI_DESCRIPTION_TEXT ${MSVC} $(DESC_MSVC)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;------------------------------------------------------------------------
;Uninstaller Sections							                                      -
;------------------------------------------------------------------------
Section "Uninstall"

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

; The file to write
OutFile "QLandkarteGT.exe"

