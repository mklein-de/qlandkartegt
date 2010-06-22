//C-  -*- C++ -*-
//C- -------------------------------------------------------------------
//C- Copyright (c) 2009 Marc Feld
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, either version 2 of the license,
//C- or (at your option) any later version. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C-
//C- This program is distributed in the hope that it will be useful,
//C- but WITHOUT ANY WARRANTY; without even the implied warranty of
//C- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//C- GNU General Public License for more details.
//C-  ------------------------------------------------------------------

#include "CActions.h"
#include <QAction>
#include <QDebug>
#include "CMainWindow.h"
#include "CMenus.h"
#include "CCanvas.h"
#include "CSearchDB.h"
#include "CWptDB.h"
#include "CMapDB.h"
#include "CTrackDB.h"
#include "CDiaryDB.h"
#include "CLiveLogDB.h"
#include "COverlayDB.h"
#include "CRouteDB.h"
#include "CTrackToolWidget.h"
#include "CCreateMapGeoTiff.h"
#include "CMainWindow.h"
#include "CResources.h"
#include "IDevice.h"
#include "CDlgCreateWorldBasemap.h"
#include "CUndoStackModel.h"
#include "CUndoStackView.h"
#include "CTrackUndoCommandDeletePts.h"
#include "CTrackUndoCommandPurgePts.h"
#ifdef PLOT_3D
#include "CMap3D.h"
#endif

CActions::CActions(QObject *parent) :
QObject(parent), parent(parent)
{
    actionGroup = (CMenus*) parent;
    canvas = theMainWindow->getCanvas();
    createAction(tr("F1"), ":/icons/iconMap16x16", tr("&Map ..."), "aSwitchToMap", tr("Manage maps."));
    createAction(tr("F2"), ":/icons/iconWaypoint16x16", tr("&Waypoint ..."), "aSwitchToWpt", tr("Manage waypoints."));
    createAction(tr("F3"), ":/icons/iconTrack16x16", tr("&Track ..."), "aSwitchToTrack", tr("Manage tracks."));
    createAction(tr("F4"), ":/icons/iconRoute16x16", tr("&Route ..."), "aSwitchToRoute", tr(""));
    createAction(tr("F5"), ":/icons/iconLiveLog16x16", tr("Live &Log ..."), "aSwitchToLiveLog", tr("Toggle live log recording."));
    createAction(tr("F6"), ":/icons/iconOverlay16x16", tr("&Overlay ..."), "aSwitchToOverlay", tr("Manage overlays, such as textboxes"));
    createAction(tr("F7"), ":/icons/iconGlobe+16x16", tr("Mor&e ..."), "aSwitchToMainMore", tr("Extended functions."));
    createAction(tr("F8"), ":/icons/iconClear16x16", tr("&Clear all"), "aClearAll", tr("Remove all waypoints, tracks, ..."));
    createAction(tr("F9"), ":/icons/iconUpload16x16", tr("U&pload all"), "aUploadAll", tr("Upload all data to device."));
    createAction(tr("F10"), ":/icons/iconDownload16x16", tr("Down&load all"), "aDownloadAll", tr("Download all data from device."));

    createAction(tr("ESC"), ":/icons/iconBack16x16", tr("&Back"), "aSwitchToMain", tr("Go back to main menu."));
    createAction(tr("F1"), ":/icons/iconMoveMap16x16", tr("Mo&ve Map"), "aMoveArea", tr("Move the map. Press down the left mouse button and move the mouse."));
    createAction(tr("F2"), ":/icons/iconZoomArea16x16", tr("&Zoom Map"), "aZoomArea", tr("Select area for zoom."));
    createAction(tr("F3"), ":/icons/iconCenter16x16", tr("&Center Map"), "aCenterMap", tr("Find your map by jumping to it's center."));

    createAction(tr("F5"), ":/icons/iconSelect16x16", tr("Select &Sub Map"), "aSelectArea", tr("Select area of map to export. Select area by pressing down the left mouse button and move the mouse."));
    createAction(tr("F6"), ":/icons/iconEdit16x16", tr("&Edit / Create Map"), "aEditMap", tr(""));
    createAction(tr("F7"), ":/icons/iconFind16x16", tr("&Search Map"), "aSearchMap", tr("Find symbols on a map via image recognition."));
#ifdef PLOT_3D
    createAction(tr("F8"),":/icons/icon3D16x16.png",tr("3&D Map..."), "aSwitchToMap3D", tr("Show 3D map"));
#endif
    createAction(tr("F9"), ":/icons/iconUpload16x16", tr("U&pload"), "aUploadMap", tr("Upload map selection to device."));

#ifdef PLOT_3D
    createAction(tr("ESC"),":/icons/iconBack16x16",tr("&Close"),"aCloseMap3D",tr("Close 3D view."));
    createAction(tr("F1"),0,tr("3D / 2D"),"aMap3DMode",tr("Toggle between 3D and 2D map."));
    createAction(tr("F2"),0,tr("FPV / Rot."),"aMap3DFPVMode",tr("Toggle between first person view and rotation mode."));
    createAction(tr("F3"),":/icons/iconLight16x16",tr("Lighting On/Off"), "aMap3DLighting",tr("Turn on/off lighting."));
    createAction(tr("F4"),":/icons/iconTrack16x16",tr("Trackmode"), "aMap3DTrackMode",tr("Glue point of view to track."));
#endif
    //
    createAction(tr("F5"), ":/icons/iconAdd16x16", tr("&New Waypoint"), "aNewWpt", tr("Create a new user waypoint. The default position will be the current cursor position."));
    createAction(tr("F6"), ":/icons/iconEdit16x16", tr("&Edit Waypoint"), "aEditWpt", tr("Switch cursor to 'Edit Waypoint' mode. Point-n-click to edit a waypoint."));
    createAction(tr("F7"), ":/icons/iconMove16x16", tr("&Move Waypoint"), "aMoveWpt", tr("Switch cursor to 'Move Waypoint' mode. Point-click-move-click to move a waypoint. Use the right mouse button to abort. It is ok to leave 'Move Waypoint' mode and to resume."));
#ifdef HAS_EXIF
    createAction(tr("F8"),":/icons/iconRaster16x16",tr("From &Images..."),"aImageWpt",tr("Create waypoints from geo-referenced images in a path."));
#endif
    createAction(tr("F9"), ":/icons/iconUpload16x16", tr("U&pload"), "aUploadWpt", tr("Upload waypoints to device."));
    createAction(tr("F10"), ":/icons/iconDownload16x16", tr("Down&load"), "aDownloadWpt", tr("Download waypoints from device."));
    //
    createAction(tr("F5"), ":/icons/iconAdd16x16", tr("Join &Tracks"), "aCombineTrack", tr("Join multiple selected tracks to one."));
    createAction(tr("F6"), ":/icons/iconEdit16x16", tr("&Edit Track"), "aEditTrack", tr("Toggle track edit dialog."));
    createAction(tr("F7"), ":/icons/iconEditCut16x16", tr("&Split Track"), "aCutTrack", tr("Split a track into pieces."));
    createAction(tr("F8"), ":/icons/iconSelect16x16", tr("&Select Points"), "aSelTrack", tr("Select track points by rectangle."));
    createAction(tr("F9"), ":/icons/iconUpload16x16", tr("U&pload"), "aUploadTrack", tr("Upload tracks to device."));
    createAction(tr("F10"), ":/icons/iconDownload16x16", tr("Down&load"), "aDownloadTrack", tr("Download tracks from device."));
    createAction(tr("Del"), ":/icons/iconClear16x16", tr("Purge Selection"), "aTrackPurgeSelection", tr("Purge the selected points of the track."));
    createAction(tr("ctrl+Del"), ":/icons/iconClear16x16", tr("Delete Selection"), "aDeleteTrackSelection", tr("Deletes the selected points of the track."));
    //
    createAction(tr("F5"), ":/icons/iconPlayPause16x16", tr("&Start / Stop"), "aLiveLog", tr("Start / stop live log recording."));
    createAction(tr("F6"), ":/icons/iconLock16x16", tr("Move Map to &Pos."), "aLockMap", tr("Move the map to keep the positon cursor centered."));
    createAction(tr("F7"), ":/icons/iconAdd16x16", tr("Add &Waypoint"), "aAddWpt", tr("Add a waypoint at current position."));
    //

    createAction(tr("F5"), ":/icons/iconText16x16", tr("Add Static &Text Box"), "aText", tr("Add text on the map."));
    createAction(tr("F6"), ":/icons/iconTextBox16x16", tr("Add &Geo-Ref. Text Box"), "aTextBox", tr("Add a textbox on the map."));
    createAction(tr("F7"), ":/icons/iconDistance16x16", tr("Add Distance &Polyline"), "aDistance", tr("Add a polyline to measure distances."));
    //
    createAction(tr("F5"), ":/icons/iconDiary16x16", tr("&Diary"), "aDiary", tr("Add / edit diary data"));
    createAction(tr("F6"), ":/icons/iconColorChooser16x16", tr("&Pick Color"), "aColorPicker", tr("test only"));
    createAction(tr("F7"), 0, tr("Create World &Basemap"), "aWorldBasemap", tr("Create a world basemap from OSM tiles to be used by QLandkarte M"));

    //
    createAction(tr("F9"), ":/icons/iconUpload16x16", tr("U&pload"), "aUploadRoute", tr("Upload routes to device."));
    createAction(tr("F10"), ":/icons/iconDownload16x16", tr("Down&load"), "aDownloadRoute", tr("Download routes from device."));
    //

    createAction(tr("+"), ":/icons/zoomin.png", tr("&Zoom in"), "aZoomIn", tr("Zoom's into the Map."));
    createAction(tr("-"), ":/icons/zoomout.png", tr("&Zoom out"), "aZoomOut", tr("Zoom's out of the Map."));
    createAction(QKeySequence (Qt::ALT + Qt::Key_Left).toString(), ":/icons/editcopy.png", tr("&Move left"), "aMoveLeft", tr("Move to the left side."));
    createAction(QKeySequence (Qt::ALT + Qt::Key_Right).toString(), ":/icons/editcopy.png", tr("&Move right"), "aMoveRight", tr("Move to the right side."));
    createAction(QKeySequence (Qt::ALT + Qt::Key_Up).toString(), ":/icons/editcopy.png", tr("&Move up"), "aMoveUp", tr("Move up."));
    createAction(QKeySequence (Qt::ALT + Qt::Key_Down).toString(), ":/icons/editcopy.png", tr("&Move down"), "aMoveDown", tr("Move down."));
    createAction(tr("ctrl+c"), ":/icons/editcopy.png", tr("&Copy"), "aCopyToClipboard", tr("Copy selected trackpoints to clipboard."));
    createAction(tr("ctrl+v"), ":/icons/editpaste.png", tr("&Paste"), "aPasteFromClipboard", tr("Paste Track."));
    createAction(tr("ctrl+z"), ":/icons/editpaste.png", tr("&Undo"), "aUndo", tr("Undo a command."));
    createAction(tr("ctrl+y"), ":/icons/editpaste.png", tr("&Redo"), "aRedo", tr("Redo a command."));
}


CActions::~CActions()
{

}


void CActions::createAction(const QString& shortCut,
const char * icon,
const QString& name,
const QString& actionName,
const QString& toolTip)
{
    if (findChild<QAction *> (actionName))
    {
        qDebug()
            << tr("Action with the name '%1' already registered. Please choose an other name.").arg(actionName);
        return;
    }

    QAction *tmpAction = new QAction(QIcon(icon), name, this);
    tmpAction->setObjectName(actionName);
    tmpAction->setShortcut(shortCut);
    tmpAction->setToolTip(toolTip);

    QString slotName;
    if (actionName.startsWith('a'))
        slotName = "func" + actionName.mid(1);
    else
        slotName = actionName;

    connect(tmpAction, SIGNAL(triggered()), this, QString("1" + slotName + "()").toAscii().data());

}


QAction *CActions::getAction(const QString& actionObjectName)
{
    QAction *tmpAction = findChild<QAction *> (actionObjectName);
    if (tmpAction)
        return tmpAction;
    else
    {
        qDebug()
            << tr("Action with name '%1' not found. %2").arg(actionObjectName).arg(Q_FUNC_INFO);
        return new QAction(this);
    }
}


void CActions::setMenuTitle(const QString& title)
{
    menuTitle = title;
}


void CActions::setMenuPixmap(const QPixmap& pixmap)
{
    menuPixmap = pixmap;
}


void CActions::funcSwitchToMain()
{
    // qDebug() << Q_FUNC_INFO;
    setMenuTitle(tr("&Main"));
    setMenuPixmap(QPixmap(":/icons/backGlobe128x128"));
    actionGroup->switchToActionGroup(CMenus::MainMenu);
    funcMoveArea();
}


void CActions::funcSwitchToMap()
{
    // qDebug() << Q_FUNC_INFO;
    setMenuTitle(tr("&Maps"));
    setMenuPixmap(QPixmap(":/icons/backMap128x128"));
    actionGroup->switchToActionGroup(CMenus::MapMenu);
    CMapDB::self().gainFocus();
    funcMoveArea();
}


void CActions::funcSwitchToMap3D()
{
#ifdef PLOT_3D
    setMenuTitle(tr("&Maps"));
    setMenuPixmap(QPixmap(":/icons/backMap128x128"));
    actionGroup->switchToActionGroup(CMenus::Map3DMenu);
    CMapDB::self().gainFocus();
    CMapDB::self().show3DMap(true);
#endif
}


void CActions::funcSwitchToWpt()
{
    setMenuTitle(tr("&Waypoints"));
    setMenuPixmap(QPixmap(":/icons/backWaypoint128x128"));
    actionGroup->switchToActionGroup(CMenus::WptMenu);
    CWptDB::self().gainFocus();
    funcMoveArea();
}


void CActions::funcSwitchToTrack()
{
    setMenuTitle(tr("&Tracks"));
    setMenuPixmap(QPixmap(":/icons/backTrack128x128"));
    actionGroup->switchToActionGroup(CMenus::TrackMenu);
    CTrackDB::self().gainFocus();
    funcMoveArea();
}


void CActions::funcSwitchToRoute()
{
    setMenuTitle(tr("&Routes"));
    setMenuPixmap(QPixmap(":/icons/backRoute128x128"));
    actionGroup->switchToActionGroup(CMenus::RouteMenu);
    CRouteDB::self().gainFocus();
    funcMoveArea();
}


void CActions::funcSwitchToLiveLog()
{
    setMenuTitle(tr("&Live Log"));
    setMenuPixmap(QPixmap(":/icons/backLiveLog128x128"));
    actionGroup->switchToActionGroup(CMenus::LiveLogMenu);
    CLiveLogDB::self().gainFocus();
    funcMoveArea();
}


void CActions::funcSwitchToOverlay()
{
    setMenuTitle(tr("&Overlay"));
    setMenuPixmap(QPixmap(":/icons/backOverlay128x128"));
    actionGroup->switchToActionGroup(CMenus::OverlayMenu);
    COverlayDB::self().gainFocus();
    funcMoveArea();
}


void CActions::funcSwitchToMainMore()
{
    setMenuTitle(tr("&Main (More)"));
    setMenuPixmap(QPixmap(":/icons/backGlobe+128x128"));
    actionGroup->switchToActionGroup(CMenus::MainMoreMenu);
    funcMoveArea();
}


void CActions::funcDiary()
{
    CDiaryDB::self().openEditWidget();
}


void CActions::funcColorPicker()
{
    canvas->setMouseMode(CCanvas::eMouseColorPicker);
}


void CActions::funcClearAll()
{
    theMainWindow->clearAll();
}


void CActions::funcUploadAll()
{
    IDevice * dev = CResources::self().device();
    if (dev == 0)
        return;

    dev->uploadAll();
}


void CActions::funcDownloadAll()
{
    IDevice * dev = CResources::self().device();
    if (dev == 0)
        return;

    dev->downloadAll();
}


void CActions::funcMoveArea()
{
    canvas->setMouseMode(CCanvas::eMouseMoveArea);
}


void CActions::funcZoomArea()
{
    canvas->setMouseMode(CCanvas::eMouseZoomArea);
}


void CActions::funcCenterMap()
{
    canvas->move(CCanvas::eMoveCenter);
}


void CActions::funcSelectArea()
{
    canvas->setMouseMode(CCanvas::eMouseSelectArea);
}


void CActions::funcEditMap()
{

    CMapDB::self().editMap();
    if (CCreateMapGeoTiff::self())
    {
        // setEnabled(false); // not finished
        connect(CCreateMapGeoTiff::self(), SIGNAL(destroyed(QObject*)), this, SLOT(slotEnable()));
    }
}


void CActions::funcSearchMap()
{
    CMapDB::self().searchMap();
}


void CActions::funcUploadMap()
{
    QStringList keys;
    CMapDB::self().upload(keys);
}


void CActions::funcNewWpt()
{
    canvas->setMouseMode(CCanvas::eMouseAddWpt);
}


void CActions::funcCloseMap3D()
{
#ifdef PLOT_3D
    //qDebug() << Q_FUNC_INFO;
    CMapDB::self().show3DMap(false);
    setMenuTitle(tr("Maps ..."));
    setMenuPixmap(QPixmap(":/icons/backMap128x128"));
    actionGroup->switchToActionGroup(CMenus::MapMenu);
    CMapDB::self().gainFocus();
    funcMoveArea();
#endif
}


void CActions::funcMap3DLighting()
{
#ifdef PLOT_3D
    CMap3D * map = CMapDB::self().getMap3D();
    map->lightTurn();
#endif
}


void CActions::funcMap3DMode()
{
#ifdef PLOT_3D
    CMap3D * map = CMapDB::self().getMap3D();
    if(map)
    {
        map->slotChange3DMode();
    }
#endif
}


void CActions::funcMap3DFPVMode()
{
#ifdef PLOT_3D
    CMap3D * map = CMapDB::self().getMap3D();
    if(map)
    {
        map->slotChange3DFPVMode();
    }
#endif
}


void CActions::funcMap3DTrackMode()
{
#ifdef PLOT_3D
    CMap3D * map = CMapDB::self().getMap3D();
    if(map)
    {
        map->slotChange3DTrackMode();
    }
#endif
}


void CActions::funcEditWpt()
{
    canvas->setMouseMode(CCanvas::eMouseEditWpt);
}


void CActions::funcMoveWpt()
{
    canvas->setMouseMode(CCanvas::eMouseMoveWpt);
}


void CActions::funcImageWpt()
{
#ifdef HAS_EXIF
    CWptDB::self().createWaypointsFromImages();
#endif
}


void CActions::funcUploadWpt()
{
    QStringList keys;
    CWptDB::self().upload(keys);
}


void CActions::funcDownloadWpt()
{
    CWptDB::self().download();
}


void CActions::funcEditTrack()
{
    CTrackToolWidget * toolview = CTrackDB::self().getToolWidget();
    if (toolview)
        toolview->slotEdit();
}


void CActions::funcCombineTrack()
{
    CTrackDB::self().CombineTracks();
}


void CActions::funcCutTrack()
{
    canvas->setMouseMode(CCanvas::eMouseCutTrack);
}


void CActions::funcSelTrack()
{
    canvas->setMouseMode(CCanvas::eMouseSelTrack);
}


void CActions::funcUploadTrack()
{
    QStringList keys;
    CTrackDB::self().upload(keys);
}


void CActions::funcDownloadTrack()
{
    CTrackDB::self().download();
}


void CActions::funcTrackPurgeSelection()
{
    qDebug() << "funcTrackPurgeSelection";
    CTrack *track = CTrackDB::self().highlightedTrack();
    if (track)
    {
        CUndoStackModel::getInstance()->push(new CTrackUndoCommandPurgePts(track));
    }
}


void CActions::funcDeleteTrackSelection()
{
    qDebug() << "funcDeleteTrackSelection";
    CTrack *track = CTrackDB::self().highlightedTrack();
    if (track)
    {
        CUndoStackModel::getInstance()->push(new CTrackUndoCommandPurgePts(track));
    }
}


void CActions::funcUploadRoute()
{
    QStringList keys;
    CRouteDB::self().upload(keys);
}


void CActions::funcDownloadRoute()
{
    CRouteDB::self().download();
}


void CActions::funcLiveLog()
{
    CLiveLogDB::self().start(!CLiveLogDB::self().logging());
}


void CActions::funcLockMap()
{
    CLiveLogDB::self().setLockToCenter(!CLiveLogDB::self().lockToCenter());
}


void CActions::funcAddWpt()
{
    CLiveLogDB::self().addWpt();
}


void CActions::funcText()
{
    canvas->setMouseMode(CCanvas::eMouseAddText);
}


void CActions::funcTextBox()
{
    canvas->setMouseMode(CCanvas::eMouseAddTextBox);
}


void CActions::funcDistance()
{
    canvas->setMouseMode(CCanvas::eMouseAddDistance);
}


void CActions::funcWorldBasemap()
{
    CDlgCreateWorldBasemap dlg;
    dlg.exec();
}


//    else if(e->key() == Qt::Key_Plus) {
void CActions::funcZoomIn()
{
    canvas->zoom(true, canvas->geometry().center());
}


//    else if(e->key() == Qt::Key_Minus) {
void CActions::funcZoomOut()
{
    canvas->zoom(false, canvas->geometry().center());
}


//    else if(e->key() == Qt::Key_Left) {
void CActions::funcMoveLeft()
{
    canvas->move(CCanvas::eMoveLeft);
}


//    else if(e->key() == Qt::Key_Right) {
void CActions::funcMoveRight()
{
    canvas->move(CCanvas::eMoveRight);
}


//    else if(e->key() == Qt::Key_Up) {
void CActions::funcMoveUp()
{
    canvas->move(CCanvas::eMoveUp);
}


//    else if(e->key() == Qt::Key_Down) {
void CActions::funcMoveDown()
{
    canvas->move(CCanvas::eMoveDown);
}


//    else if (e->key() == Qt::Key_C && e->modifiers() == Qt::ControlModifier)
void CActions::funcCopyToClipboard()
{
    CTrackDB::self().copyToClipboard();
}


//    else if (e->key() == Qt::Key_V && e->modifiers() == Qt::ControlModifier)
void CActions::funcPasteFromClipboard()
{
    CTrackDB::self().pasteFromClipboard();
}


void CActions::funcRedo()
{
    CUndoStackModel::getInstance()->redo();
    //    emit CTrackDB::m_self.sigChanged();
    //    emit CTrackDB::m_self->sigModified();
}


void CActions::funcUndo()
{
    CUndoStackModel::getInstance()->undo();
    //    emit CTrackDB::m_self()->sigChanged();
    //    emit CTrackDB::m_self()->sigModified();
}
