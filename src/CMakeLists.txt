include( ${QT_USE_FILE})

#set(CMAKE_VERBOSE_MAKEFILE off)

set(qlandkarte_SRCS
    main.cpp
    mitab.cpp
    tcxreader.cpp
    printpreview.cpp
    GeoMath.cpp
    WptIcons.cpp
    CResources.cpp
    CCopyright.cpp
    CMainWindow.cpp
    CCanvas.cpp
    CStatusDEM.cpp
    IMouse.cpp
    CMouseMoveMap.cpp
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
    IMap.cpp
    CMapNoMap.cpp
    CMapDEM.cpp
    CMapFile.cpp
    CMapLevel.cpp
    CMapQMAP.cpp
    CMapQMAPExport.cpp
    CMapRaster.cpp
    CMapGeoTiff.cpp
    CMapSelection.cpp
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
    CQlb.cpp
    CGpx.cpp
    IDevice.cpp
    CDeviceTBDOE.cpp
    CDeviceGarmin.cpp
    CCreateMapOSM.cpp
    CCreateMapQMAP.cpp
    CCreateMapGeoTiff.cpp
    CCreateMapGridTool.cpp
    CMapDB.cpp
    CMapToolWidget.cpp
    CMapEditWidget.cpp
    CTrack.cpp
    CTrackDB.cpp
    CTrackToolWidget.cpp
    CTrackEditWidget.cpp
    ITrackStat.cpp
    CTrackStatWidget.cpp
    CTrackStatProfileWidget.cpp
    CTrackStatSpeedWidget.cpp
    CTrackStatTraineeWidget.cpp
    CTabWidget.cpp
    CDiary.cpp
    CDiaryDB.cpp
    CDiaryEditWidget.cpp
    CPlot.cpp
    CPlotAxis.cpp
    CPlotData.cpp
    CLiveLog.cpp
    CLiveLogDB.cpp
    CLiveLogToolWidget.cpp
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
    CPicProcess.cpp
    CImage.cpp
    IGarminStrTbl.cpp
    CGarminStrTbl8.cpp
    CGarminPolygon.cpp
    CGarminPoint.cpp
)

IF(WIN32)
    SET(qlandkarte_SRCS ${qlandkarte_SRCS} qlandkarte.rc)
ENDIF(WIN32)

set(qlandkarte_HDRS
    Platform.h
    version.h
    mitab.h
    tcxreader.h
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
    IMap.h
    CMapNoMap.h
    CMapDEM.h
    CMapFile.h
    CMapLevel.h
    CMapQMAP.h
    CMapQMAPExport.h
    CMapRaster.h
    CMapGeoTiff.h
    CMapSelection.h
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
    CQlb.h
    CGpx.h
    IDevice.h
    CDeviceTBDOE.h
    CDeviceGarmin.h
    CCreateMapOSM.h
    CCreateMapQMAP.h
    CCreateMapGeoTiff.h
    CCreateMapGridTool.h
    CMapDB.h
    CMapToolWidget.h
    CMapEditWidget.h
    CTrack.h
    CTrackDB.h
    CTrackToolWidget.h
    CTrackEditWidget.h
    ITrackStat.h
    CTrackStatWidget.h
    CTrackStatProfileWidget.h
    CTrackStatSpeedWidget.h
    CTrackStatTraineeWidget.h
    CTabWidget.h
    CDiary.h
    CDiaryDB.h
    CDiaryEditWidget.h
    CPlot.h
    CPlotAxis.h
    CPlotData.h
    CLiveLog.h
    CLiveLogDB.h
    CLiveLogToolWidget.h
    IOverlay.h
    COverlayDB.h
    COverlayToolWidget.h
    COverlayText.h
    COverlayTextBox.h
    COverlayDistance.h
    IUnit.h
    CUnitMetric.h
    CUnitImperial.h
    CUnitNautic.h
    CPicProcess.h
    CImage.h
    IGarminStrTbl.h
    CGarminStrTbl8.h
    CGarminPolygon.h
    CGarminPoint.h
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
    ICreateMapOSM.ui
    ICreateMapQMAP.ui
    ICreateMapGeoTiff.ui
    ICreateMapGridTool.ui
    IStatusDEM.ui
    IDlgProjWizzard.ui
    IDiaryEditWidget.ui
    ILiveLogToolWidget.ui
    IOverlayToolWidget.ui

)

SET(qlandkarte_RCS
    resources.qrc
)

SET (PLOT_3D OFF CACHE BOOL "this feature isn't finished yet, so disable by default")
SET (QL_OPENGL_INCLUDE_DIR "")
SET (QL_OPENGL_LIBRARIES "")
IF(PLOT_3D AND OPENGL_FOUND)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLOT_3D")
    SET(qlandkarte_HDRS ${qlandkarte_HDRS} CTrack3DWidget.h)
    SET(qlandkarte_SRCS ${qlandkarte_SRCS} CTrack3DWidget.cpp)
    SET(QL_OPENGL_INCLUDE_DIR ${QT_QTOPENGL_INCLUDE_DIR})
    SET(QL_OPENGL_LIBRARIES ${QT_QTOPENGL_LIBRARIES})
ENDIF(PLOT_3D AND OPENGL_FOUND)

SET (GARMIN OFF CACHE BOOL "this feature isn't finished yet, so disable by default")
IF(GARMIN)
    add_definitions(-DGARMIN)
    SET(qlandkarte_HDRS ${qlandkarte_HDRS} CMapTDB.h CGarminTile.h Garmin.h)
    SET(qlandkarte_SRCS ${qlandkarte_SRCS} CMapTDB.cpp CGarminTile.cpp)
ENDIF(GARMIN)

qt4_wrap_cpp(qlandkarte_MOC_SRCS ${qlandkarte_HDRS})
qt4_wrap_ui(qlandkarte_UI_HDRS ${qlandkarte_UIS})
qt4_add_resources(qlandkarte_RC_SRCS ${qlandkarte_RCS} )

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${QT_QTXML_INCLUDE_DIR} ${QT_QTOPENGL_INCLUDE_DIR} ${QL_OPENGL_INCLUDE_DIR} ../include)

if(CMAKE_HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif(CMAKE_HAVE_STDINT_H)

if(CMAKE_HAVE_INTTYPES_H)
    add_definitions(-DHAVE_INTTYPES_H)
endif(CMAKE_HAVE_INTTYPES_H)

if(CMAKE_HAVE_BYTESWAP_H)
    add_definitions(-DHAVE_BYTESWAP_H)
endif(CMAKE_HAVE_BYTESWAP_H)

if(CMAKE_HAVE_BIGENDIAN)
    add_definitions(-DHAVE_BIGENDIAN)
endif(CMAKE_HAVE_BIGENDIAN)

#TODO test for alignment
add_definitions(-DCAN_UNALIGNED)

add_executable( qlandkartegt ${qlandkarte_SRCS} ${qlandkarte_HDRS} ${qlandkarte_UI_HDRS} ${qlandkarte_RC_SRCS} ${qlandkarte_MOC_SRCS})

IF(UNIX)
  add_definitions(-Wall -O2 -DQL_LIBDIR=${QL_LIBDIR} -DSOEXT=${SOEXT})
  target_link_libraries(qlandkartegt gdal proj ${QT_LIBRARIES} ${QT_QTNETWORK_LIBRARIES} ${QT_QTXML_LIBRARIES} ${QL_OPENGL_LIBRARIES})
ENDIF(UNIX)

IF(WIN32)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /subsystem:windows")
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  target_link_libraries(qlandkartegt gdal_i proj_i qtmain.lib ${QT_LIBRARIES} ${QT_QTNETWORK_LIBRARIES} ${QT_QTXML_LIBRARIES} ${QL_OPENGL_LIBRARIES} ${OPENGL_LIBRARIES})
ENDIF(WIN32)



