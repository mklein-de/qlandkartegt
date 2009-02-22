/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/

#include "CMegaMenu.h"
#include "CCanvas.h"
#include "CSearchDB.h"
#include "CWptDB.h"
#include "CMapDB.h"
#include "CTrackDB.h"
#include "CDiaryDB.h"
#include "CLiveLogDB.h"
#include "COverlayDB.h"
#include "CTrackToolWidget.h"
#include "CCreateMapGeoTiff.h"
#include "CMainWindow.h"
#include "CResources.h"
#include "IDevice.h"

#ifdef PLOT_3D
#include "CMap3DWidget.h"
#endif

#include <QtGui>
/// Enhanced QLabel used by CMegaMenu
class CLabel : public QLabel
{
    public:
        CLabel(QWidget * parent): QLabel(parent) {
            setMouseTracking(true);
        }
        ~CLabel(){}

        void mouseMoveEvent(QMouseEvent * e) {
            setBackgroundRole(QPalette::Highlight);
            setForegroundRole(QPalette::HighlightedText);
        }

        void leaveEvent(QEvent *) {
            setBackgroundRole(QPalette::Window);
            setForegroundRole(QPalette::WindowText);
        }

        void paintEvent(QPaintEvent * e) {

            QPainter p;
            p.begin(this);
            if(backgroundRole() == QPalette::Highlight) {
                p.setBrush(QBrush( QColor(66,121,206,150) ));
            }
            else {
                p.setBrush(QBrush( QColor(66,121,206,0) ));
            }
            p.setPen(Qt::NoPen);
            p.drawRect(rect());
            p.end();

            QLabel::paintEvent(e);
        }
};


CMegaMenu * CMegaMenu::m_self = 0;

/// Left hand side multi-function menu
CMegaMenu::CMegaMenu(CCanvas * canvas)
: QLabel(canvas)
, canvas(canvas)
, current(0)
, fsMain(11, func_key_state_t())
, fsMap(11, func_key_state_t())
, fsMap3D(11, func_key_state_t())
, fsWpt(11, func_key_state_t())
, fsTrack(11, func_key_state_t())
, fsLiveLog(11, func_key_state_t())
, fsOverlay(11, func_key_state_t())
, fsMainMore(11, func_key_state_t())
{

    fsMain[0] = func_key_state_t(0,tr("-"),0,tr(""));
    fsMain[1] = func_key_state_t(":/icons/iconMap16x16",tr("Map ..."),&CMegaMenu::funcSwitchToMap,tr("Manage maps."));
    fsMain[2] = func_key_state_t(":/icons/iconWaypoint16x16",tr("Waypoint ..."),&CMegaMenu::funcSwitchToWpt,tr("Manage waypoints."));
    fsMain[3] = func_key_state_t(":/icons/iconTrack16x16",tr("Track ..."),&CMegaMenu::funcSwitchToTrack,tr("Manage tracks."));
    fsMain[4] = func_key_state_t(0,tr("-"),0,tr(""));
    fsMain[5] = func_key_state_t(":/icons/iconLiveLog16x16",tr("Live Log ..."),&CMegaMenu::funcSwitchToLiveLog,tr("Toggle live log recording."));
    fsMain[6] = func_key_state_t(":/icons/iconOverlay16x16",tr("Overlay ..."),&CMegaMenu::funcSwitchToOverlay,tr("Manage overlays, such as textboxes"));
    fsMain[7] = func_key_state_t(":/icons/iconGlobe+16x16",tr("More ..."),&CMegaMenu::funcSwitchToMainMore,tr("Extended functions."));
    fsMain[8] = func_key_state_t(":/icons/iconClear16x16",tr("Clear all"),&CMegaMenu::funcClearAll,tr("Remove all waypoints, tracks, ..."));
    fsMain[9] = func_key_state_t(":/icons/iconUpload16x16",tr("Upload all"),&CMegaMenu::funcUploadAll,tr("Upload all data to device."));
    fsMain[10] = func_key_state_t(":/icons/iconDownload16x16",tr("Download all"),&CMegaMenu::funcDownloadAll,tr("Download all data from device."));


    fsMap[0] = func_key_state_t(":/icons/iconBack16x16",tr("Back"),&CMegaMenu::funcSwitchToMain,tr("Go back to main menu."));
    fsMap[1] = func_key_state_t(":/icons/iconMoveMap16x16",tr("Move Map"),&CMegaMenu::funcMoveArea,tr("Move the map. Press down the left mouse button and move the mouse."));
    fsMap[2] = func_key_state_t(":/icons/iconZoomArea16x16",tr("Zoom Map"),&CMegaMenu::funcZoomArea,tr("Select area for zoom."));
    fsMap[3] = func_key_state_t(":/icons/iconCenter16x16",tr("Center Map"),&CMegaMenu::funcCenterMap,tr("Find your map by jumping to it's center."));
    fsMap[4] = func_key_state_t(0,tr("-"),0,tr(""));
    fsMap[5] = func_key_state_t(":/icons/iconSelect16x16",tr("Select Sub Map"),&CMegaMenu::funcSelectArea,tr("Select area of map to export. Select area by pressing down the left mouse button and move the mouse."));
    fsMap[6] = func_key_state_t(":/icons/iconEdit16x16",tr("Edit / Create Map"),&CMegaMenu::funcEditMap,tr(""));
    fsMap[7] = func_key_state_t(":/icons/iconFind16x16",tr("Search Map"),&CMegaMenu::funcSearchMap,tr("Find symbols on a map via image recognition."));
#ifdef PLOT_3D
    fsMap[8] = func_key_state_t(":/icons/icon3D16x16.png",tr("3D Map..."), &CMegaMenu::funcSwitchToMap3D, tr("Show 3D map"));
#endif
    fsMap[9] = func_key_state_t(":/icons/iconUpload16x16",tr("Upload"),&CMegaMenu::funcUploadMap,tr("Upload map selection to device."));
    fsMap[10] = func_key_state_t(0,tr("-"),0,tr(""));

#ifdef PLOT_3D
    fsMap3D[0] = func_key_state_t(":/icons/iconBack16x16",tr("Close"),&CMegaMenu::funcCloseMap3D,tr("Close 3D view."));
    fsMap3D[1] = func_key_state_t(0,tr("Flat / 3D Mode"),&CMegaMenu::funcMap3DMode,tr("Toggle between 3D track only and full map surface model."));
    fsMap3D[2] = func_key_state_t(":/icons/iconInc16x16",tr("Inc. Elevation"),&CMegaMenu::funcMap3DZoomPlus,tr("Make elevations on the map higher as they are."));
    fsMap3D[3] = func_key_state_t(":/icons/iconDec16x16",tr("Dec. Elevation"),&CMegaMenu::funcMap3DZoomMinus,tr("Make elevations on the map lower as they are."));
    fsMap3D[4] = func_key_state_t(":/icons/iconLight16x16",tr("Lighting On/Off"), &CMegaMenu::funcMap3DLighting,tr("Turn on/off lighting."));
    fsMap3D[5] = func_key_state_t(0,tr("-"),0,tr(""));
    fsMap3D[6] = func_key_state_t(0,tr("-"),0,tr(""));
    fsMap3D[7] = func_key_state_t(0,tr("-"),0,tr(""));
    fsMap3D[8] = func_key_state_t(0,tr("-"),0,tr(""));
    fsMap3D[9] = func_key_state_t(0,tr("-"),0,tr(""));
    fsMap3D[10] = func_key_state_t(0,tr("-"),0,tr(""));
#endif

    fsWpt[0] = func_key_state_t(":/icons/iconBack16x16",tr("Back"),&CMegaMenu::funcSwitchToMain,tr("Go back to main menu."));
    fsWpt[1] = func_key_state_t(":/icons/iconMoveMap16x16",tr("Move Map"),&CMegaMenu::funcMoveArea,tr("Move the map. Press down the left mouse button and move the mouse."));
    fsWpt[2] = func_key_state_t(":/icons/iconZoomArea16x16",tr("Zoom Map"),&CMegaMenu::funcZoomArea,tr("Select area for zoom."));
    fsWpt[3] = func_key_state_t(":/icons/iconCenter16x16",tr("Center Map"),&CMegaMenu::funcCenterMap,tr("Find your map by jumping to it's center."));
    fsWpt[4] = func_key_state_t(0,tr("-"),0,tr(""));
    fsWpt[5] = func_key_state_t(":/icons/iconAdd16x16",tr("New Waypoint"),&CMegaMenu::funcNewWpt,tr("Create a new user waypoint. The default position will be the current cursor position."));
    fsWpt[6] = func_key_state_t(":/icons/iconEdit16x16",tr("Edit Waypoint"),&CMegaMenu::funcEditWpt,tr("Switch cursor to 'Edit Waypoint' mode. Point-n-click to edit a waypoint."));
    fsWpt[7] = func_key_state_t(":/icons/iconWptMove16x16",tr("Move Waypoint"),&CMegaMenu::funcMoveWpt,tr("Switch cursor to 'Move Waypoint' mode. Point-click-move-click to move a waypoint. Use the right mouse button to abort. It is ok to leave 'Move Waypoint' mode and to resume."));
    fsWpt[8] = func_key_state_t(0,tr("-"),0,tr(""));
    fsWpt[9] = func_key_state_t(":/icons/iconUpload16x16",tr("Upload"),&CMegaMenu::funcUploadWpt,tr("Upload waypoints to device."));
    fsWpt[10] = func_key_state_t(":/icons/iconDownload16x16",tr("Download"),&CMegaMenu::funcDownloadWpt,tr("Download waypoints from device."));

    fsTrack[0] = func_key_state_t(":/icons/iconBack16x16",tr("Back"),&CMegaMenu::funcSwitchToMain,tr("Go back to main menu."));
    fsTrack[1] = func_key_state_t(":/icons/iconMoveMap16x16",tr("Move Map"),&CMegaMenu::funcMoveArea,tr("Move the map. Press down the left mouse button and move the mouse."));
    fsTrack[2] = func_key_state_t(":/icons/iconZoomArea16x16",tr("Zoom Map"),&CMegaMenu::funcZoomArea,tr("Select area for zoom."));
    fsTrack[3] = func_key_state_t(":/icons/iconCenter16x16",tr("Center Map"),&CMegaMenu::funcCenterMap,tr("Find your map by jumping to it's center."));
    fsTrack[4] = func_key_state_t(0,tr("-"),0,tr(""));
    fsTrack[5] = func_key_state_t(":/icons/iconAdd16x16",tr("Combine Tracks"),&CMegaMenu::funcCombineTrack,tr("Combine multiple selected tracks to one."));
    fsTrack[6] = func_key_state_t(":/icons/iconEdit16x16",tr("Edit Track"),&CMegaMenu::funcEditTrack,tr("Toggle track edit dialog."));
    fsTrack[7] = func_key_state_t(":/icons/iconEditCut16x16",tr("Cut Tracks"),&CMegaMenu::funcCutTrack,tr("Cut a track into pieces."));
    fsTrack[8] = func_key_state_t(":/icons/iconSelect16x16",tr("Select Points"),&CMegaMenu::funcSelTrack,tr("Select track points by rectangle."));
    fsTrack[9] = func_key_state_t(":/icons/iconUpload16x16",tr("Upload"),&CMegaMenu::funcUploadTrack,tr("Upload tracks to device."));
    fsTrack[10] = func_key_state_t(":/icons/iconDownload16x16",tr("Download"),&CMegaMenu::funcDownloadTrack,tr("Download tracks from device."));

    fsLiveLog[0] = func_key_state_t(":/icons/iconBack16x16",tr("Back"),&CMegaMenu::funcSwitchToMain,tr("Go back to main menu."));
    fsLiveLog[1] = func_key_state_t(":/icons/iconMoveMap16x16",tr("Move Map"),&CMegaMenu::funcMoveArea,tr("Move the map. Press down the left mouse button and move the mouse."));
    fsLiveLog[2] = func_key_state_t(":/icons/iconZoomArea16x16",tr("Zoom Map"),&CMegaMenu::funcZoomArea,tr("Select area for zoom."));
    fsLiveLog[3] = func_key_state_t(":/icons/iconCenter16x16",tr("Center Map"),&CMegaMenu::funcCenterMap,tr("Find your map by jumping to it's center."));
    fsLiveLog[4] = func_key_state_t(0,tr("-"),0,tr(""));
    fsLiveLog[5] = func_key_state_t(":/icons/iconPlayPause16x16",tr("Start / Stop"),&CMegaMenu::funcLiveLog,tr("Start / stop live log recording."));
    fsLiveLog[6] = func_key_state_t(":/icons/iconLock16x16",tr("Move Map to Pos."),&CMegaMenu::funcLockMap,tr("Move the map to keep the positon cursor centered."));
    fsLiveLog[7] = func_key_state_t(":/icons/iconAdd16x16",tr("Add Waypoint"),&CMegaMenu::funcAddWpt,tr("Add a waypoint at current position."));
    fsLiveLog[8] = func_key_state_t(0,tr("-"),0,tr(""));
    fsLiveLog[9] = func_key_state_t(0,tr("-"),0,tr(""));
    fsLiveLog[10] = func_key_state_t(0,tr("-"),0,tr(""));

    fsOverlay[0] = func_key_state_t(":/icons/iconBack16x16",tr("Back"),&CMegaMenu::funcSwitchToMain,tr("Go back to main menu."));
    fsOverlay[1] = func_key_state_t(":/icons/iconMoveMap16x16",tr("Move Map"),&CMegaMenu::funcMoveArea,tr("Move the map. Press down the left mouse button and move the mouse."));
    fsOverlay[2] = func_key_state_t(":/icons/iconZoomArea16x16",tr("Zoom Map"),&CMegaMenu::funcZoomArea,tr("Select area for zoom."));
    fsOverlay[3] = func_key_state_t(":/icons/iconCenter16x16",tr("Center Map"),&CMegaMenu::funcCenterMap,tr("Find your map by jumping to it's center."));
    fsOverlay[4] = func_key_state_t(0,tr("-"),0,tr(""));
    fsOverlay[5] = func_key_state_t(":/icons/iconText16x16",tr("Add Static Text Box"),&CMegaMenu::funcText,tr("Add text on the map."));
    fsOverlay[6] = func_key_state_t(":/icons/iconTextBox16x16",tr("Add Geo-Ref. Text Box"),&CMegaMenu::funcTextBox,tr("Add a textbox on the map."));
    fsOverlay[7] = func_key_state_t(":/icons/iconDistance16x16",tr("Add Distance Polyline"),&CMegaMenu::funcDistance,tr("Add a polyline to measure distances."));
    fsOverlay[8] = func_key_state_t(0,tr("-"),0,tr(""));
    fsOverlay[9] = func_key_state_t(0,tr("-"),0,tr(""));
    fsOverlay[10] = func_key_state_t(0,tr("-"),0,tr(""));

    fsMainMore[0] = func_key_state_t(":/icons/iconBack16x16",tr("Back"),&CMegaMenu::funcSwitchToMain,tr("Go back to main menu."));
    fsMainMore[1] = func_key_state_t(":/icons/iconMoveMap16x16",tr("Move Map"),&CMegaMenu::funcMoveArea,tr("Move the map. Press down the left mouse button and move the mouse."));
    fsMainMore[2] = func_key_state_t(":/icons/iconZoomArea16x16",tr("Zoom Map"),&CMegaMenu::funcZoomArea,tr("Select area for zoom."));
    fsMainMore[3] = func_key_state_t(":/icons/iconCenter16x16",tr("Center Map"),&CMegaMenu::funcCenterMap,tr("Find your map by jumping to it's center."));
    fsMainMore[4] = func_key_state_t(0,tr("-"),0,tr(""));
    fsMainMore[5] = func_key_state_t(":/icons/iconDiary16x16",tr("Diary"),&CMegaMenu::funcDiary,tr("Add / edit diary data"));
    fsMainMore[6] = func_key_state_t(":/icons/iconColorChooser16x16",tr("Pick Color"),&CMegaMenu::funcColorPicker ,tr("test only"));
    fsMainMore[7] = func_key_state_t(0,tr("-"),0,tr(""));
    fsMainMore[8] = func_key_state_t(0,tr("-"),0,tr(""));
    fsMainMore[9] = func_key_state_t(0,tr("-"),0,tr(""));
    fsMainMore[10] = func_key_state_t(0,tr("-"),0,tr(""));

    m_self = this;
    setScaledContents(true);

    int i;
    QVBoxLayout * mainLayout = new QVBoxLayout(this);

    QHBoxLayout * titleLayout = new QHBoxLayout();
    mainLayout->addLayout(titleLayout);

    menuTitle = new QLabel(this);
    menuTitle->setAlignment(Qt::AlignCenter);
    titleLayout->addWidget(menuTitle);

    layout = new QGridLayout();
    mainLayout->addLayout(layout);

    keyEsc   = new QLabel("Esc",this);
    layout->addWidget(keyEsc,0,0);
    keyF1   = new QLabel("F1",this);
    layout->addWidget(keyF1,1,0);
    keyF2 = new QLabel("F2",this);
    layout->addWidget(keyF2,2,0);
    keyF3 = new QLabel("F3",this);
    layout->addWidget(keyF3,3,0);
    keyF4 = new QLabel("F4",this);
    layout->addWidget(keyF4,4,0);
    keyF5 = new QLabel("F5",this);
    layout->addWidget(keyF5,5,0);
    keyF6 = new QLabel("F6",this);
    layout->addWidget(keyF6,6,0);
    keyF7 = new QLabel("F7",this);
    layout->addWidget(keyF7,7,0);
    keyF8 = new QLabel("F8",this);
    layout->addWidget(keyF8,8,0);
    keyF9 = new QLabel("F9",this);
    layout->addWidget(keyF9,9,0);
    keyF10 = new QLabel("F10",this);
    layout->addWidget(keyF10,10,0);

    for(i=0; i<11; ++i) {
        icons[i] = new QLabel(this);
        layout->addWidget(icons[i],i,1);
    }

    for(i=0; i<11; ++i) {
        names[i] = new CLabel(this);
        names[i]->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
        layout->addWidget(names[i],i,2);
    }

}


CMegaMenu::~CMegaMenu()
{

}


void CMegaMenu::switchState(QVector<func_key_state_t>* statedef)
{
    unsigned i;



    for(i=0; i<11; ++i) {
        if((*statedef)[i].icon) {
            icons[i]->setPixmap(QPixmap((*statedef)[i].icon));
        }
        else {
            icons[i]->setPixmap(QPixmap());
        }
        names[i]->setText((*statedef)[i].name);
        names[i]->setToolTip((*statedef)[i].tooltip);
    }

    current = statedef;
}


void CMegaMenu::switchByKeyWord(const QString& key)
{
    if(!isEnabled()) return;


    if(key == "Main") {
        funcSwitchToMain();
    }
    else if(key == "Waypoints" && current != &fsWpt) {
        funcSwitchToWpt();
        funcMoveArea();
    }
    else if(key == "Search" && current != &fsMain) {
        funcSwitchToMain();
        funcMoveArea();
    }
    else if(key == "Maps" && current != &fsMap) {
        funcSwitchToMap();
        funcMoveArea();
    }
    else if(key == "Tracks" && current != &fsTrack) {
        funcSwitchToTrack();
        funcMoveArea();
    }
    else if(key == "LiveLog" && current != &fsLiveLog) {
        funcSwitchToLiveLog();
        funcMoveArea();
    }
    else if(key == "Overlay" && current != &fsOverlay) {
        funcSwitchToOverlay();
        funcMoveArea();
    }

}


void CMegaMenu::keyPressEvent(QKeyEvent * e)
{
    if(!isEnabled()) return;

    if((e->key() >= Qt::Key_F1) && (e->key() < Qt::Key_F11)) {
        unsigned i = e->key() - Qt::Key_F1 + 1;
        if((*current)[i].func) {
            (this->*(*current)[i].func)();

        }
        return e->accept();
    }
    else if(e->key() == Qt::Key_Escape) {
        if((*current)[0].func) {
            (this->*(*current)[0].func)();
        }
        return e->accept();
    }
    else if(e->key() == Qt::Key_Plus) {
        canvas->zoom(true, canvas->geometry().center());
        return e->accept();
    }
    else if(e->key() == Qt::Key_Minus) {
        canvas->zoom(false, canvas->geometry().center());
        return e->accept();
    }
    else if(e->key() == Qt::Key_Left) {
        canvas->move(CCanvas::eMoveLeft);
        return e->accept();
    }
    else if(e->key() == Qt::Key_Right) {
        canvas->move(CCanvas::eMoveRight);
        return e->accept();
    }
    else if(e->key() == Qt::Key_Up) {
        canvas->move(CCanvas::eMoveUp);
        return e->accept();
    }
    else if(e->key() == Qt::Key_Down) {
        canvas->move(CCanvas::eMoveDown);
        return e->accept();
    }
}


void CMegaMenu::mousePressEvent(QMouseEvent * e)
{
    unsigned i;

    if(e->button() != Qt::LeftButton) return;

    for(i=0; i<11; ++i) {
        if(names[i]->geometry().contains(e->pos())) {
            if((*current)[i].func) {
                (this->*(*current)[i].func)();
            }
            return;
        }
    }
}


void CMegaMenu::funcSwitchToMain()
{

    menuTitle->setText(tr("<b>Main ...</b>"));
    setPixmap(QPixmap(":/icons/backGlobe128x128"));
    switchState(&fsMain);
    funcMoveArea();
}


void CMegaMenu::funcSwitchToMap()
{
    menuTitle->setText(tr("<b>Maps ...</b>"));
    setPixmap(QPixmap(":/icons/backMap128x128"));
    switchState(&fsMap);
    CMapDB::self().gainFocus();
    funcMoveArea();
}


#ifdef PLOT_3D
void CMegaMenu::funcSwitchToMap3D()
{
    menuTitle->setText(tr("<b>Maps 3D ...</b>"));
    setPixmap(QPixmap(":/icons/backMap128x128"));
    switchState(&fsMap3D);
    CMapDB::self().gainFocus();
    CMapDB::self().show3DMap(true);
}
#endif

void CMegaMenu::funcSwitchToWpt()
{
    menuTitle->setText(tr("<b>Waypoints ...</b>"));
    setPixmap(QPixmap(":/icons/backWaypoint128x128"));
    switchState(&fsWpt);
    CWptDB::self().gainFocus();
    funcMoveArea();
}


void CMegaMenu::funcSwitchToTrack()
{
    menuTitle->setText(tr("<b>Tracks ...</b>"));
    setPixmap(QPixmap(":/icons/backTrack128x128"));
    switchState(&fsTrack);
    CTrackDB::self().gainFocus();
    funcMoveArea();
}


void CMegaMenu::funcSwitchToLiveLog()
{
    menuTitle->setText(tr("<b>Live Log ...</b>"));
    setPixmap(QPixmap(":/icons/backLiveLog128x128"));
    switchState(&fsLiveLog);
    CLiveLogDB::self().gainFocus();
    funcMoveArea();
}


void CMegaMenu::funcSwitchToOverlay()
{
    menuTitle->setText(tr("<b>Overlay ...</b>"));
    setPixmap(QPixmap(":/icons/backOverlay128x128"));
    switchState(&fsOverlay);
    COverlayDB::self().gainFocus();
    funcMoveArea();
}


void CMegaMenu::funcSwitchToMainMore()
{
    menuTitle->setText(tr("<b>Main (More) ...</b>"));
    setPixmap(QPixmap(":/icons/backGlobe+128x128"));
    switchState(&fsMainMore);
    funcMoveArea();
}


void CMegaMenu::funcDiary()
{
    CDiaryDB::self().openEditWidget();
}

void CMegaMenu::funcColorPicker()
{
    canvas->setMouseMode(CCanvas::eMouseColorPicker);
}


void CMegaMenu::funcClearAll()
{
    theMainWindow->clearAll();
}


void CMegaMenu::funcUploadAll()
{
    IDevice * dev = CResources::self().device();
    if(dev == 0) return;

    dev->uploadAll();
}


void CMegaMenu::funcDownloadAll()
{
    IDevice * dev = CResources::self().device();
    if(dev == 0) return;

    dev->downloadAll();
}


void CMegaMenu::funcMoveArea()
{
    canvas->setMouseMode(CCanvas::eMouseMoveArea);
}

void CMegaMenu::funcZoomArea()
{
    canvas->setMouseMode(CCanvas::eMouseZoomArea);
}

void CMegaMenu::funcCenterMap()
{
    canvas->move(CCanvas::eMoveCenter);
}


void CMegaMenu::funcSelectArea()
{
    canvas->setMouseMode(CCanvas::eMouseSelectArea);
}


void CMegaMenu::funcEditMap()
{

    CMapDB::self().editMap();
    if(CCreateMapGeoTiff::self()) {
        setEnabled(false);
        connect(CCreateMapGeoTiff::self(), SIGNAL(destroyed(QObject*)), this, SLOT(slotEnable()));
    }
}


void CMegaMenu::funcSearchMap()
{
    CMapDB::self().searchMap();
}

void CMegaMenu::funcUploadMap()
{
    CMapDB::self().upload();
}


void CMegaMenu::funcNewWpt()
{
    canvas->setMouseMode(CCanvas::eMouseAddWpt);
}


#ifdef PLOT_3D
void CMegaMenu::funcCloseMap3D()
{
    CMapDB::self().show3DMap(false);
    menuTitle->setText(tr("<b>Maps ...</b>"));
    setPixmap(QPixmap(":/icons/backMap128x128"));
    switchState(&fsMap);
    CMapDB::self().gainFocus();
    funcMoveArea();
}

void CMegaMenu::funcMap3DZoomPlus()
{
    CMap3DWidget * map = CMapDB::self().getMap3D();
    if(map){
        map->eleZoomIn();
    }
}

void CMegaMenu::funcMap3DZoomMinus()
{
    CMap3DWidget * map = CMapDB::self().getMap3D();
    if(map){
        map->eleZoomOut();
    }
}

void CMegaMenu::funcMap3DLighting()
{
    CMap3DWidget * map = CMapDB::self().getMap3D();
    map->lightTurn();
}

void CMegaMenu::funcMap3DMode()
{
    CMap3DWidget * map = CMapDB::self().getMap3D();
    if(map){
        map->changeMode();
    }
}
#endif

void CMegaMenu::funcEditWpt()
{
    canvas->setMouseMode(CCanvas::eMouseEditWpt);
}


void CMegaMenu::funcMoveWpt()
{
    canvas->setMouseMode(CCanvas::eMouseMoveWpt);
}


void CMegaMenu::funcUploadWpt()
{
    CWptDB::self().upload();
}


void CMegaMenu::funcDownloadWpt()
{
    CWptDB::self().download();
}


void CMegaMenu::funcEditTrack()
{
    CTrackToolWidget * toolview = CTrackDB::self().getToolWidget();
    if(toolview) toolview->slotEdit();
}


void CMegaMenu::funcCombineTrack()
{
    CTrackDB::self().CombineTracks();
}


void CMegaMenu::funcCutTrack()
{
    canvas->setMouseMode(CCanvas::eMouseCutTrack);
}


void CMegaMenu::funcSelTrack()
{
    canvas->setMouseMode(CCanvas::eMouseSelTrack);
}


void CMegaMenu::funcUploadTrack()
{
    CTrackDB::self().upload();
}


void CMegaMenu::funcDownloadTrack()
{
    CTrackDB::self().download();
}


void CMegaMenu::funcLiveLog()
{
    CLiveLogDB::self().start(!CLiveLogDB::self().logging());
}


void CMegaMenu::funcLockMap()
{
    CLiveLogDB::self().setLockToCenter(!CLiveLogDB::self().lockToCenter());
}


void CMegaMenu::funcAddWpt()
{
    CLiveLogDB::self().addWpt();
}


void CMegaMenu::funcText()
{
    canvas->setMouseMode(CCanvas::eMouseAddText);
}


void CMegaMenu::funcTextBox()
{
    canvas->setMouseMode(CCanvas::eMouseAddTextBox);
}


void CMegaMenu::funcDistance()
{
    canvas->setMouseMode(CCanvas::eMouseAddDistance);
}
