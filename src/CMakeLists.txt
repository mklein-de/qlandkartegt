include( ${QT_USE_FILE})

set(CMAKE_VERBOSE_MAKEFILE ON)

set(qlandkarte_SRCS
    main.cpp
    mitab.cpp
    printpreview.cpp
    GeoMath.cpp
    WptIcons.cpp
    CResources.cpp
    CCopyright.cpp
    CMainWindow.cpp
    CCanvas.cpp
    CStatusCanvas.cpp
    IMouse.cpp
    CMouseMoveMap.cpp
    CMouseSelMap.cpp
    CMouseAddWpt.cpp
    CMouseMoveWpt.cpp
    CMouseEditWpt.cpp
    CMouseRefPoint.cpp
    IMap.cpp
    CMapDEM.cpp
    CMapFile.cpp
    CMapLevel.cpp
    CMapQMAP.cpp
    CMapRaster.cpp
    CMegaMenu.cpp
    IDB.cpp
    CWpt.cpp
    CWptDB.cpp
    CWptToolWidget.cpp
    CSearchDB.cpp
    CSearchToolWidget.cpp
    CDlgConfig.cpp
    CDlgEditWpt.cpp
    CDlgWptIcon.cpp
    CDlgEditMapLevel.cpp
    CDlgProjWizzard.cpp
    CQlb.cpp
    CGpx.cpp
    IDevice.cpp
    CDeviceTBDOE.cpp
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
    CTabWidget.cpp
    CDiary.cpp
    CDiaryDB.cpp
    CDiaryEditWidget.cpp
)

set(qlandkarte_HDRS
    version.h
    mitab.h
    printpreview.h
    GeoMath.h
    WptIcons.h
    CResources.h
    CCopyright.h
    CMainWindow.h
    CCanvas.h
    CStatusCanvas.h
    IMouse.h
    CMouseMoveMap.h
    CMouseSelMap.h
    CMouseAddWpt.h
    CMouseMoveWpt.h
    CMouseEditWpt.h
    CMouseRefPoint.h
    IMap.h
    CMapNoMap.h
    CMapDEM.h
    CMapFile.h
    CMapLevel.h
    CMapQMAP.h
    CMapRaster.h
    CMegaMenu.h
    IDB.h
    CWpt.h
    CWptDB.h
    CWptToolWidget.h
    CSearchDB.h
    CSearchToolWidget.h
    CDlgConfig.h
    CDlgEditWpt.h
    CDlgWptIcon.h
    CDlgEditMapLevel.h
    CDlgProjWizzard.h
    CQlb.h
    CGpx.h
    IDevice.h
    CDeviceTBDOE.h
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
    CTabWidget.h
    CDiary.h
    CDiaryDB.h
    CDiaryEditWidget.h
)

SET(qlandkarte_UIS
    ICopyright.ui
    IWptToolWidget.ui
    IMapToolWidget.ui
    IMapEditWidget.ui
    ITrackToolWidget.ui
    ITrackEditWidget.ui
    ISearchToolWidget.ui
    IDlgConfig.ui
    IDlgEditWpt.ui
    IDlgWptIcon.ui
    IDlgEditMapLevel.ui
    ICreateMapOSM.ui
    ICreateMapQMAP.ui
    ICreateMapGeoTiff.ui
    ICreateMapGridTool.ui
    IStatusCanvas.ui
    IDlgProjWizzard.ui
    IDiaryEditWidget.ui
)

SET(qlandkarte_RCS
    resources.qrc
)

qt4_wrap_cpp(qlandkarte_MOC_SRCS ${qlandkarte_HDRS})
qt4_wrap_ui(qlandkarte_UI_HDRS ${qlandkarte_UIS})
qt4_add_resources(qlandkarte_RC_SRCS ${qlandkarte_RCS} )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable( qlandkartegt ${qlandkarte_SRCS} ${qlandkarte_HDRS} ${qlandkarte_UI_HDRS} ${qlandkarte_RC_SRCS} ${qlandkarte_MOC_SRCS})

IF(UNIX)
  add_definitions(-Wall -O2)
  target_link_libraries(qlandkartegt gdal proj ${QT_LIBRARIES} ${QT_QTNETWORK_LIBRARIES} ${QT_QTXML_LIBRARIES})
ENDIF(UNIX)

IF(WIN32)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /subsystem:windows")
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  target_link_libraries(qlandkartegt gdal_i proj_i qtmain.lib ${QT_LIBRARIES} ${QT_QTNETWORK_LIBRARIES} ${QT_QTXML_LIBRARIES})
ENDIF(WIN32)



