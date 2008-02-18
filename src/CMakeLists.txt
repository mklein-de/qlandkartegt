include( ${QT_USE_FILE})
#include( ${QT_QTNETWORK_INCLUDE_DIR})
#include( ${QT_QTXML_INCLUDE_DIR})

set(CMAKE_VERBOSE_MAKEFILE ON)

set(qlandkarte_SRCS
    main.cpp
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
    IMap.cpp
    CMapDEM.cpp
    CMapFile.cpp
    CMapLevel.cpp
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
    CDlgCreateMap.cpp
    CDlgEditMapLevel.cpp
    CQlb.cpp
    CGpx.cpp
    IDevice.cpp
    CDeviceTBDOE.cpp
    CCreateMapOSM.cpp
    CCreateMapQMAP.cpp
    CMapDB.cpp
    CMapToolWidget.cpp
    CTrackDB.cpp
    CTrackToolWidget.cpp
)

set(qlandkarte_HDRS
    version.h
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
    IMap.h
    CMapNoMap.h
    CMapDEM.h
    CMapFile.h
    CMapLevel.h
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
    CDlgCreateMap.h
    CDlgEditMapLevel.h
    CQlb.h
    CGpx.h
    IDevice.h
    CDeviceTBDOE.h
    CCreateMapOSM.h
    CCreateMapQMAP.h
    CMapDB.h
    CMapToolWidget.h
    CTrackDB.h
    CTrackToolWidget.h
)

SET(qlandkarte_UIS
    ICopyright.ui
    IWptToolWidget.ui
    IMapToolWidget.ui
    ISearchToolWidget.ui
    IDlgConfig.ui
    IDlgEditWpt.ui
    IDlgWptIcon.ui
    IDlgCreateMap.ui
    IDlgEditMapLevel.ui
    ICreateMapOSM.ui
    ICreateMapQMAP.ui
    IStatusCanvas.ui
)

SET(qlandkarte_RCS
    resources.qrc
)

qt4_wrap_cpp(qlandkarte_MOC_SRCS ${qlandkarte_HDRS})
qt4_wrap_ui(qlandkarte_UI_HDRS ${qlandkarte_UIS})
qt4_add_resources(qlandkarte_RC_SRCS ${qlandkarte_RCS} )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable( QLandkarteGT ${qlandkarte_SRCS} ${qlandkarte_HDRS} ${qlandkarte_UI_HDRS} ${qlandkarte_RC_SRCS} ${qlandkarte_MOC_SRCS})

if(UNIX)
  add_definitions(-Wall -O2)
endif(UNIX)

#target_link_libraries(QLandkarteGT gdal proj ${QT_LIBRARIES} ${QT_QTNETWORK_LIBRARIES} ${QT_QTXML_LIBRARIES})


IF(UNIX)
  add_definitions(-Wall -O2)
  target_link_libraries(QLandkarteGT gdal proj ${QT_LIBRARIES} ${QT_QTNETWORK_LIBRARIES} ${QT_QTXML_LIBRARIES})
ENDIF(UNIX)

IF(WIN32)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  target_link_libraries(QLandkarteGT gdal_i proj_i ${QT_LIBRARIES} ${QT_QTNETWORK_LIBRARIES} ${QT_QTXML_LIBRARIES})
ENDIF(WIN32)



