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
  RequestExecutionLevel user  

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


;------------------------------------------------------------------------
; Component add		   	   					                                      -
;------------------------------------------------------------------------
;Components description

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
  SectionEnd
  LangString DESC_QLandkarteGT ${LANG_ENGLISH} "This is a GeoTiff viewer for the PC"
  
  Section "FWTools" FWTools
  	SetOutPath $INSTDIR
  	File FWTools\*.dll
  	File FWTools\*.exe
  	; Don't do it if we can package install
  	;NSISdl::download http://home.gdal.org/fwtools/FWTools210.exe $TEMP\FWTools210.exe
  	;Pop $R0 ;Get the return value
  	;  StrCmp $R0 "success" +3
  	;    MessageBox MB_OK "Download failed: $R0"
  	;    Quit
  	;ExecWait '"$TEMP\FWTools210.exe"'
  SectionEnd
  LangString DESC_FWTools ${LANG_ENGLISH} "FWTools includes OpenEV, GDAL, MapServer, PROJ.4 and OGDI as well as some supporting components."
  
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

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
   !insertmacro MUI_DESCRIPTION_TEXT ${QLandkarteGT} $(DESC_QLandkarteGT)
   !insertmacro MUI_DESCRIPTION_TEXT ${FWTools} $(DESC_FWTools)
   !insertmacro MUI_DESCRIPTION_TEXT ${QT} $(DESC_QT)
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

SectionEnd

; The file to write
OutFile "QLandkarteGT.exe"

