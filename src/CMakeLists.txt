include( ${QT_USE_FILE})
include( ${QT_QTNETWORK_INCLUDE_DIR})
include( ${QT_QTXML_INCLUDE_DIR})

set(CMAKE_VERBOSE_MAKEFILE ON)

set(qlandkarte_SRCS
    main.cpp
    GeoMath.cpp
    WptIcons.cpp
    CResources.cpp
    CCopyright.cpp
    CMainWindow.cpp
    CCanvas.cpp
    IMouse.cpp
    CMouseMoveMap.cpp
    CMouseSelMap.cpp
    CMouseAddWpt.cpp
    IMap.cpp
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
    CQlb.cpp
    CGpx.cpp
    IDevice.cpp
    CDeviceTBDOE.cpp
    CCreateMapOSM.cpp
)

set(qlandkarte_HDRS
    version.h
    GeoMath.h
    WptIcons.h
    CResources.h
    CCopyright.h
    CMainWindow.h
    CCanvas.h
    IMouse.h
    CMouseMoveMap.h
    CMouseSelMap.h
    CMouseAddWpt.h
    IMap.h
    CMapNoMap.h
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
    CQlb.h
    CGpx.h
    IDevice.h
    CDeviceTBDOE.h
    CCreateMapOSM.h
)

SET(qlandkarte_UIS
    ICopyright.ui
    IWptToolWidget.ui
    ISearchToolWidget.ui
    IDlgConfig.ui
    IDlgEditWpt.ui
    IDlgWptIcon.ui
    IDlgCreateMap.ui
    ICreateMapOSM.ui
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

target_link_libraries(QLandkarteGT gdal proj ${QT_LIBRARIES} ${QT_QTNETWORK_LIBRARIES} ${QT_QTXML_LIBRARIES})



