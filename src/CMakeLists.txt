project(qlandkarteqt-binary)

find_package(Qt4 REQUIRED)
find_package(OpenGL)

find_package(GDAL REQUIRED)
find_package(PROJ REQUIRED)
find_package(EXIF)
find_package(DMTX)


include(${QT_USE_FILE})

set(qlandkarte_SRCS
    main.cpp
    mitab.cpp
    tcxreader.cpp
    CActions.cpp
    CMenus.cpp
    printpreview.cpp
    GeoMath.cpp
    WptIcons.cpp
    CResources.cpp
    CCopyright.cpp
    CMainWindow.cpp
    CCanvas.cpp
    CCanvasUndoCommandZoom.cpp
    CStatusDEM.cpp
    IMouse.cpp
    CMouseMoveMap.cpp
    CMouseZoomMap.cpp
    CMouseSelMap.cpp
    CMouseAddWpt.cpp
    CMouseMoveWpt.cpp
    CMouseEditWpt.cpp
    CMouseRefPoint.cpp
    CMouseCutTrack.cpp
    CMouseSelTrack.cpp
    CMouseAddText.cpp
    CMouseAddTextBox.cpp
    CMouseAddDistance.cpp
    CMouseOverlay.cpp
    CMouseColorPicker.cpp
    IMap.cpp
    CMapNoMap.cpp
    CMapOSM.cpp
    CMapDEM.cpp
    CMapFile.cpp
    CMapLevel.cpp
    CMapQMAP.cpp
    CMapQMAPExport.cpp
    CMapRaster.cpp
    CMapGeoTiff.cpp
    IMapSelection.cpp
    CMapSelectionRaster.cpp
    CMapSelectionGarmin.cpp
    CMapSearchWidget.cpp
    CMapSearchCanvas.cpp
    CMapSearchThread.cpp
    CMegaMenu.cpp
    IDB.cpp
    CWpt.cpp
    CWptDB.cpp
    CWptToolWidget.cpp
    CSearch.cpp
    CSearchDB.cpp
    CSearchToolWidget.cpp
    CDlgConfig.cpp
    CDlgEditWpt.cpp
    CDlgWptIcon.cpp
    CDlgEditMapLevel.cpp
    CDlgCombineTracks.cpp
    CDlgProjWizzard.cpp
    CDlgEditText.cpp
    CDlgEditDistance.cpp
    CDlgCreateWorldBasemap.cpp
    CDlgDelWpt.cpp
    CDlgConvertToTrack.cpp
    CDlgEditRoute.cpp
    CDlgMapTDBConfig.cpp
    CDlgScreenshot.cpp
    CDlgTrackFilter.cpp
    CDlgSetupGarminIcons.cpp
    CDlgConfig3D.cpp
    CQlb.cpp
    CGpx.cpp
    IDevice.cpp
    CDeviceQLandkarteM.cpp
    CDeviceGarmin.cpp
    CDeviceMikrokopter.cpp
    CDeviceNMEA.cpp
    CCreateMapOSM.cpp
    CCreateMapQMAP.cpp
    CCreateMapGeoTiff.cpp
    CCreateMapGridTool.cpp
    CMapUndoCommandMove.cpp
    CMapDB.cpp
    CMapToolWidget.cpp
    CMapEditWidget.cpp
    CUndoStackModel.cpp
    CUndoStackView.cpp
    CTrackUndoCommandDelete.cpp
    CTrackUndoCommandDeletePts.cpp
    CTrackUndoCommandPurgePts.cpp
    CTrackUndoCommandSelect.cpp
    CTrack.cpp
    CTrackDB.cpp
    CTrackToolWidget.cpp
    CTrackEditWidget.cpp
    ITrackStat.cpp
    CTrackStatProfileWidget.cpp
    CTrackStatSpeedWidget.cpp
    CTrackStatTraineeWidget.cpp
    CRoute.cpp
    CRouteDB.cpp
    CRouteToolWidget.cpp
    CTabWidget.cpp
    CDiary.cpp
    CDiaryDB.cpp
    CDiaryEditWidget.cpp
    CPlot.cpp
    CPlotAxis.cpp
    CPlotAxisTime.cpp
    CPlotData.cpp
    CLiveLog.cpp
    CLiveLogDB.cpp
    CLiveLogToolWidget.cpp
    COsmTilesHash.cpp
    IOverlay.cpp
    COverlayDB.cpp
    COverlayToolWidget.cpp
    COverlayText.cpp
    COverlayTextBox.cpp
    COverlayDistance.cpp
    IUnit.cpp
    CUnitMetric.cpp
    CUnitImperial.cpp
    CUnitNautic.cpp
    CImage.cpp
    IGarminStrTbl.cpp
    CGarminStrTbl6.cpp
    CGarminStrTbl8.cpp
    CGarminStrTblUtf8.cpp
    CGarminPolygon.cpp
    CGarminPoint.cpp
    CGarminIndex.cpp
    CGarminTile.cpp
    CGarminExport.cpp
    IGarminTyp.cpp
    CGarminTyp.cpp
    CGarminTypNT.cpp
    CMapTDB.cpp
    CHelpButton.cpp
)

if (WIN32)
    SET(qlandkarte_SRCS ${qlandkarte_SRCS} qlandkarte.rc)
endif (WIN32)

set(qlandkarte_HDRS
    Platform.h
    version.h
    mitab.h
    tcxreader.h
    CActions.h
    CMenus.h
    printpreview.h
    GeoMath.h
    WptIcons.h
    CResources.h
    CCopyright.h
    CMainWindow.h
    CCanvas.h
    CStatusDEM.h
    IMouse.h
    CMouseMoveMap.h
    CMouseZoomMap.h
    CMouseSelMap.h
    CMouseAddWpt.h
    CMouseMoveWpt.h
    CMouseEditWpt.h
    CMouseRefPoint.h
    CMouseCutTrack.h
    CMouseSelTrack.h
    CMouseAddText.h
    CMouseAddTextBox.h
    CMouseAddDistance.h
    CMouseOverlay.h
    CMouseColorPicker.h
    IMap.h
    CMapNoMap.h
    CMapOSM.h
    CMapDEM.h
    CMapFile.h
    CMapLevel.h
    CMapQMAP.h
    CMapQMAPExport.h
    CMapRaster.h
    CMapGeoTiff.h
    IMapSelection.h
    CMapSelectionRaster.h
    CMapSelectionGarmin.h
    CMapSearchWidget.h
    CMapSearchCanvas.h
    CMapSearchThread.h
    CMegaMenu.h
    IDB.h
    CWpt.h
    CWptDB.h
    CWptToolWidget.h
    CSearch.h
    CSearchDB.h
    CSearchToolWidget.h
    CDlgConfig.h
    CDlgEditWpt.h
    CDlgWptIcon.h
    CDlgEditMapLevel.h
    CDlgCombineTracks.h
    CDlgProjWizzard.h
    CDlgEditText.h
    CDlgEditDistance.h
    CDlgDelWpt.h
    CDlgCreateWorldBasemap.h
    CDlgConvertToTrack.h
    CDlgEditRoute.h
    CDlgMapTDBConfig.h
    CDlgScreenshot.h
    CDlgTrackFilter.h
    CDlgSetupGarminIcons.h
    CDlgConfig3D.h
    CQlb.h
    CGpx.h
    IDevice.h
    CDeviceQLandkarteM.h
    CDeviceGarmin.h
    CDeviceMikrokopter.h
    CDeviceNMEA.h
    CCreateMapOSM.h
    CCreateMapQMAP.h
    CCreateMapGeoTiff.h
    CCreateMapGridTool.h
    CMapDB.h
    CMapToolWidget.h
    CMapEditWidget.h
    CUndoStackModel.h
    CUndoStackView.h
    CTrackUndoCommandDelete.h
    CTrackUndoCommandSelect.h
    CTrack.h
    CTrackDB.h
    CTrackToolWidget.h
    CTrackEditWidget.h
    ITrackStat.h
    CTrackStatProfileWidget.h
    CTrackStatSpeedWidget.h
    CTrackStatTraineeWidget.h
    CRoute.h
    CRouteDB.h
    CRouteToolWidget.h
    CTabWidget.h
    CDiary.h
    CDiaryDB.h
    CDiaryEditWidget.h
    CPlot.h
    CPlotAxis.h
    CPlotAxisTime.h
    CPlotData.h
    CLiveLog.h
    CLiveLogDB.h
    CLiveLogToolWidget.h
    IOverlay.h
    COsmTilesHash.h
    COverlayDB.h
    COverlayToolWidget.h
    COverlayText.h
    COverlayTextBox.h
    COverlayDistance.h
    IUnit.h
    CUnitMetric.h
    CUnitImperial.h
    CUnitNautic.h
    CImage.h
    IGarminStrTbl.h
    CGarminStrTbl6.h
    CGarminStrTbl8.h
    CGarminStrTblUtf8.h
    CGarminPolygon.h
    CGarminPoint.h
    CGarminIndex.h
    CGarminTile.h
    Garmin.h
    CGarminExport.h
    IGarminTyp.h
    CGarminTyp.h
    CGarminTypNT.h
    CMapTDB.h
    CAppOpts.h
    CHelpButton.h
)

SET(qlandkarte_UIS
    ICopyright.ui
    IWptToolWidget.ui
    IMapToolWidget.ui
    IMapEditWidget.ui
    IMapSearchWidget.ui
    IMapQMAPExport.ui
    ITrackToolWidget.ui
    ITrackEditWidget.ui
    ITrackStatWidget.ui
    ISearchToolWidget.ui
    IDlgConfig.ui
    IDlgEditWpt.ui
    IDlgWptIcon.ui
    IDlgEditMapLevel.ui
    IDlgCombineTracks.ui
    IDlgEditDistance.ui
    IDlgDelWpt.ui
    IDlgCreateWorldBasemap.ui
    IDlgConvertToTrack.ui
    IDlgEditRoute.ui
    IDlgMapTDBConfig.ui
    IDlgScreenshot.ui
    IDlgTrackFilter.ui
    IDlgSetupGarminIcons.ui
    IDlgConfig3D.ui
    ICreateMapOSM.ui
    ICreateMapQMAP.ui
    ICreateMapGeoTiff.ui
    ICreateMapGridTool.ui
    IRouteToolWidget.ui
    IStatusDEM.ui
    IDlgProjWizzard.ui
    IDiaryEditWidget.ui
    ILiveLogToolWidget.ui
    IOverlayToolWidget.ui
    IGarminExport.ui
    IHelpDlg.ui
)

SET(qlandkarte_RCS
    resources.qrc
)

SET (PLOT_3D ON CACHE BOOL "3D Maps with OpenGL")
SET (QL_OPENGL_INCLUDE_DIR "")
SET (QL_OPENGL_LIBRARIES "")

IF(PLOT_3D AND OPENGL_FOUND)
    SET (PLOT_3D_NEW OFF CACHE BOOL "Use new 3D mode (developmental)")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLOT_3D")
    if(PLOT_3D_NEW)
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLOT_3D_NEW")
    endif(PLOT_3D_NEW)
    SET(qlandkarte_HDRS ${qlandkarte_HDRS} CMap3D.h CDlg3DHelp.h)
    SET(qlandkarte_SRCS ${qlandkarte_SRCS} CMap3D.cpp CDlg3DHelp.cpp)
    SET(qlandkarte_UIS ${qlandkarte_UIS} IDlg3DHelp.ui)    
    SET(QL_OPENGL_INCLUDE_DIR ${QT_QTOPENGL_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})
    SET(QL_OPENGL_LIBRARIES ${QT_QTOPENGL_LIBRARIES} ${OPENGL_LIBRARIES})
ENDIF(PLOT_3D AND OPENGL_FOUND)

SET (WMS_CLIENT OFF CACHE BOOL "WMS Maps service")
IF(WMS_CLIENT)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWMS_CLIENT")
    SET(qlandkarte_HDRS ${qlandkarte_HDRS} CMapWMS.h CCreateMapWMS.h)
    SET(qlandkarte_SRCS ${qlandkarte_SRCS} CMapWMS.cpp CCreateMapWMS.cpp)
    SET(qlandkarte_UIS ${qlandkarte_UIS} ICreateMapWMS.ui)    
ENDIF(WMS_CLIENT)

IF(EXIF_FOUND)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAS_EXIF")
else(EXIF_FOUND)
    set(EXIF_INCLUDE_DIRS "")
    set(EXIF_LIBRARIES "")
ENDIF(EXIF_FOUND)

IF(DMTX_FOUND)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAS_DMTX")
else(EXIF_FOUND)
    set(DMTX_INCLUDE_DIRS "")
    set(DMTX_LIBRARIES "")
ENDIF(DMTX_FOUND)

qt4_wrap_cpp(qlandkarte_MOC_SRCS ${qlandkarte_HDRS})
qt4_wrap_ui(qlandkarte_UI_HDRS ${qlandkarte_UIS})
qt4_add_resources(qlandkarte_RC_SRCS ${qlandkarte_RCS})

SET (ENABLE_TRANSLATION ON CACHE BOOL "enable translations")
#-D ENABLE_TRANSLATION:bool=OFF
IF(ENABLE_TRANSLATION)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DENABLE_TRANSLATION")

    file (GLOB TRANSLATIONS_FILES locale/*.ts)

    option (UPDATE_TRANSLATIONS "Update source translation translations/*.ts
    files (WARNING: make clean will delete the source .ts files! Danger!)")
    if (UPDATE_TRANSLATIONS)
       qt4_create_translation(qlandkarte_QM_FILES
                              ${qlandkarte_SRCS}
                              ${qlandkarte_HDRS}
                              ${qlandkarte_UIS}
                              ${TRANSLATIONS_FILES}
                          )
    else (UPDATE_TRANSLATIONS)
       qt4_add_translation(qlandkarte_QM_FILES ${TRANSLATIONS_FILES})
    endif (UPDATE_TRANSLATIONS)

    if (UNIX AND NOT WIN32 AND NOT APPLE)
 	 install(
         	 FILES
                   ${qlandkarte_QM_FILES}
    	     DESTINATION
                   ${DATA_INSTALL_PREFIX}/qlandkartegt/translations)
    endif (UNIX AND NOT WIN32 AND NOT APPLE)
    if (APPLE)
  	  add_custom_target(locversion.plist ALL
  		COMMAND
  			${CMAKE_SOURCE_DIR}/MacOSX/mklocversion.sh ${MAC_BUNDLE_PATH} ${qlandkarte_QM_FILES}
  	  )
    endif (APPLE)
ENDIF(ENABLE_TRANSLATION)

if(UNIX)
    add_definitions(-D_TTY_POSIX_)
endif(UNIX)

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${GDAL_INCLUDE_DIRS}
  ${PROJ_INCLUDE_DIRS}
  ${QT_QTXML_INCLUDE_DIR}
  ${QT_QTSQL_INCLUDE_DIR}
  ${QT_QTOPENGL_INCLUDE_DIR}
  ${QT_QTNETWORK_INCLUDE_DIR}
  ${QL_OPENGL_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/3rdparty/SerialPort
  ${CMAKE_SOURCE_DIR}/3rdparty/QMKToolBox
  ${CMAKE_SOURCE_DIR}/3rdparty/getopt
  ${EXIF_INCLUDE_DIRS}
  ${DMTX_INCLUDE_DIRS}
)

set(qlandkarte_ALLINP
    ${qlandkarte_SRCS}
    ${qlandkarte_HDRS}
    ${qlandkarte_UI_HDRS}
    ${qlandkarte_RC_SRCS}
    ${qlandkarte_MOC_SRCS}
    ${qlandkarte_QM_FILES}
)

if (APPLE)
  set(MAC_EXTRA_RES
  	  ${CMAKE_SOURCE_DIR}/MacOSX/${MAC_ICON_FILE}
  )
  set_source_files_properties(
  	${MAC_EXTRA_RES} ${qlandkarte_QM_FILES}
  PROPERTIES
  	MACOSX_PACKAGE_LOCATION Resources
  )
  add_executable(${PROGNAME} MACOSX_BUNDLE ${qlandkarte_ALLINP} ${MAC_EXTRA_RES})
  add_custom_target(mac-deploy macdeployqt "${MAC_BUNDLE_PATH}")
else(APPLE)
  add_executable(${PROGNAME} ${qlandkarte_ALLINP})
endif(APPLE)

target_link_libraries(
    ${PROGNAME}
      ${GDAL_LIBRARIES}
      ${PROJ_LIBRARIES}
      ${QT_LIBRARIES}
      ${QT_QTNETWORK_LIBRARIES}
      ${QT_QTXML_LIBRARIES}
      ${QT_QTSQL_LIBRARIES}
      ${QL_OPENGL_LIBRARIES}
      ${EXIF_LIBRARIES}
      ${DMTX_LIBRARIES}
      SerialPort
      QMKToolBox
      getopt
  )

install(
    TARGETS
      ${PROGNAME}
    DESTINATION
      ${BIN_INSTALL_DIR}
)

if (WIN32)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /subsystem:windows")
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE )
  target_link_libraries(${PROGNAME}
        ${QT_LIBRARIES}
        ${QT_QTNETWORK_LIBRARIES}
        ${QT_QTXML_LIBRARIES}
        ${QT_QTSQL_LIBRARIES}
        ${QL_OPENGL_LIBRARIES}
        ${OPENGL_LIBRARIES}
        ${QT_LIBRARY_DIR}/qtmain.lib
        SerialPort
        QMKToolBox)
endif (WIN32)

