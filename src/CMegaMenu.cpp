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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/

#include "CMegaMenu.h"
#include "CCanvas.h"
#include "CSearchDB.h"
#include "CWptDB.h"
#include "CMapDB.h"
#include "CTrackDB.h"
#include "CDiaryDB.h"
#include "CLiveLogDB.h"
#include "CTrackToolWidget.h"
#include "CCreateMapGeoTiff.h"
#include "CMainWindow.h"
#include "CResources.h"
#include "IDevice.h"

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

const CMegaMenu::func_key_state_t CMegaMenu::fsMain[] =
{
    {0,QObject::tr("-"),0,tr("")}
    ,{":/icons/iconMap16x16",QObject::tr("Map ..."),&CMegaMenu::funcSwitchToMap,tr("Manage maps.")}
    ,{":/icons/iconWaypoint16x16",QObject::tr("Waypoint ..."),&CMegaMenu::funcSwitchToWpt,tr("Manage waypoints.")}
    ,{":/icons/iconTrack16x16",QObject::tr("Track ..."),&CMegaMenu::funcSwitchToTrack,tr("Manage tracks.")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{":/icons/iconLiveLog16x16",QObject::tr("Live Log"),&CMegaMenu::funcSwitchToLiveLog,tr("Toggle live log recording.")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{":/icons/iconDiary16x16",QObject::tr("Diary"),&CMegaMenu::funcDiary,tr("Add / edit diary data")}
    ,{":/icons/iconClear16x16",QObject::tr("Clear all"),&CMegaMenu::funcClearAll,tr("Remove all waypoints, tracks, ...")}
    ,{":/icons/iconUpload16x16",QObject::tr("Upload all"),&CMegaMenu::funcUploadAll,tr("Upload all data to device.")}
    ,{":/icons/iconDownload16x16",QObject::tr("Download all"),&CMegaMenu::funcDownloadAll,tr("Download all data to device.")}

};

const CMegaMenu::func_key_state_t CMegaMenu::fsMap[] =
{
    {":/icons/iconBack16x16",QObject::tr("Back"),&CMegaMenu::funcSwitchToMain,tr("Go back to main menu.")}
    ,{":/icons/iconMoveMap16x16",QObject::tr("Move Map"),&CMegaMenu::funcMoveArea,tr("Move the map. Press down the left mouse button and move the mouse.")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{":/icons/iconCenter16x16",QObject::tr("Center Map"),&CMegaMenu::funcCenterMap,tr("Find your map by jumping to it's center.")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{":/icons/iconSelMap16x16",QObject::tr("Export Map"),&CMegaMenu::funcSelectArea,tr("Select area of map to export. Select area by pressing down the left mouse button and move the mouse.")}
    ,{":/icons/iconEdit16x16",QObject::tr("Edit / Create Map"),&CMegaMenu::funcEditMap,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
};

const CMegaMenu::func_key_state_t CMegaMenu::fsWpt[] =
{
    {":/icons/iconBack16x16",QObject::tr("Back"),&CMegaMenu::funcSwitchToMain,tr("Go back to main menu.")}
    ,{":/icons/iconMoveMap16x16",QObject::tr("Move Map"),&CMegaMenu::funcMoveArea,tr("Move the map. Press down the left mouse button and move the mouse.")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{":/icons/iconCenter16x16",QObject::tr("Center Map"),&CMegaMenu::funcCenterMap,tr("Find your map by jumping to it's center.")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{":/icons/iconAdd16x16",QObject::tr("New Waypoint"),&CMegaMenu::funcNewWpt,tr("Create a new user waypoint. The default position will be the current cursor position.")}
    ,{":/icons/iconEdit16x16",QObject::tr("Edit Waypoint"),&CMegaMenu::funcEditWpt,tr("Switch cursor to 'Edit Waypoint' mode. Point-n-click to edit a waypoint.")}
    ,{":/icons/iconWptMove16x16",QObject::tr("Move Waypoint"),&CMegaMenu::funcMoveWpt,tr("Switch cursor to 'Move Waypoint' mode. Point-click-move-click to move a waypoint. Use the right mouse button to abort. It is ok to leave 'Move Waypoint' mode and to resume.")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{":/icons/iconUpload16x16",tr("Upload"),&CMegaMenu::funcUploadWpt,tr("Upload waypoints to device.")}
    ,{":/icons/iconDownload16x16",tr("Download"),&CMegaMenu::funcDownloadWpt,tr("Download waypoints from device.")}
};

const CMegaMenu::func_key_state_t CMegaMenu::fsTrack[] =
{
    {":/icons/iconBack16x16",QObject::tr("Back"),&CMegaMenu::funcSwitchToMain,tr("Go back to main menu.")}
    ,{":/icons/iconMoveMap16x16",QObject::tr("Move Map"),&CMegaMenu::funcMoveArea,tr("Move the map. Press down the left mouse button and move the mouse.")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{":/icons/iconCenter16x16",QObject::tr("Center Map"),&CMegaMenu::funcCenterMap,tr("Find your map by jumping to it's center.")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{":/icons/iconAdd16x16",QObject::tr("Combine Tracks"),&CMegaMenu::funcCombineTrack,tr("Combine multiple selected tracks to one.")}
    ,{":/icons/iconEdit16x16",QObject::tr("Edit Track"),&CMegaMenu::funcEditTrack,tr("Toggle track edit dialog.")}
    ,{":/icons/iconEditCut16x16",QObject::tr("Cut Tracks"),&CMegaMenu::funcCutTrack,tr("Cut a track into pieces.")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{":/icons/iconDownload16x16",tr("Download"),&CMegaMenu::funcDownloadTrack,tr("Download tracks from device.")}
};

const CMegaMenu::func_key_state_t CMegaMenu::fsLiveLog[] =
{
    {":/icons/iconBack16x16",QObject::tr("Back"),&CMegaMenu::funcSwitchToMain,tr("Go back to main menu.")}
    ,{":/icons/iconMoveMap16x16",QObject::tr("Move Map"),&CMegaMenu::funcMoveArea,tr("Move the map. Press down the left mouse button and move the mouse.")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{":/icons/iconCenter16x16",QObject::tr("Center Map"),&CMegaMenu::funcCenterMap,tr("Find your map by jumping to it's center.")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{":/icons/iconPlayPause16x16",QObject::tr("Start / Stop"),&CMegaMenu::funcLiveLog,tr("Start / stop live log recording.")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
    ,{0,QObject::tr("-"),0,tr("")}
};

CMegaMenu * CMegaMenu::m_self = 0;

/// Left hand side multi-function menu
CMegaMenu::CMegaMenu(CCanvas * canvas)
: QLabel(canvas)
, canvas(canvas)
{
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

    funcSwitchToMain();
}


CMegaMenu::~CMegaMenu()
{

}


void CMegaMenu::switchState(const func_key_state_t statedef[11])
{
    unsigned i;

    for(i=0; i<11; ++i) {
        if(statedef[i].icon) {
            icons[i]->setPixmap(QPixmap(statedef[i].icon));
        }
        else {
            icons[i]->setPixmap(QPixmap());
        }
        names[i]->setText(statedef[i].name);
        names[i]->setToolTip(statedef[i].tooltip);
    }

    current = statedef;
}


void CMegaMenu::switchByKeyWord(const QString& key)
{
    if(!isEnabled()) return;

    if(key == "Waypoints" && current != fsWpt) {
        funcSwitchToWpt();
        funcMoveArea();
    }
    else if(key == "Search" && current != fsMain) {
        funcSwitchToMain();
        funcMoveArea();
    }
    else if(key == "Maps" && current != fsMap) {
        funcSwitchToMap();
        funcMoveArea();
    }
    else if(key == "Tracks" && current != fsTrack) {
        funcSwitchToTrack();
        funcMoveArea();
    }
    else if(key == "LiveLog" && current != fsLiveLog) {
        funcSwitchToLiveLog();
        funcMoveArea();
    }

}


void CMegaMenu::keyPressEvent(QKeyEvent * e)
{
    if(!isEnabled()) return;

    if((e->key() >= Qt::Key_F1) && (e->key() < Qt::Key_F11)) {
        unsigned i = e->key() - Qt::Key_F1 + 1;
        if(current[i].func) {
            (this->*current[i].func)();

        }
        return e->accept();
    }
    else if(e->key() == Qt::Key_Escape) {
        if(current[0].func) {
            (this->*current[0].func)();
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
            if(current[i].func) {
                (this->*current[i].func)();
            }
            return;
        }
    }
}


void CMegaMenu::funcSwitchToMain()
{

    menuTitle->setText(tr("<b>Main ...</b>"));
    setPixmap(QPixmap(":/icons/backGlobe128x128"));
    switchState(fsMain);
    funcMoveArea();
}


void CMegaMenu::funcSwitchToMap()
{
    menuTitle->setText(tr("<b>Maps ...</b>"));
    setPixmap(QPixmap(":/icons/backMap128x128"));
    switchState(fsMap);
    CMapDB::self().gainFocus();
    funcMoveArea();
}


void CMegaMenu::funcSwitchToWpt()
{
    menuTitle->setText(tr("<b>Waypoints ...</b>"));
    setPixmap(QPixmap(":/icons/backWaypoint128x128"));
    switchState(fsWpt);
    CWptDB::self().gainFocus();
    funcMoveArea();
}


void CMegaMenu::funcSwitchToTrack()
{
    menuTitle->setText(tr("<b>Tracks ...</b>"));
    setPixmap(QPixmap(":/icons/backTrack128x128"));
    switchState(fsTrack);
    CTrackDB::self().gainFocus();
    funcMoveArea();
}


void CMegaMenu::funcSwitchToLiveLog()
{
    menuTitle->setText(tr("<b>Live Log ...</b>"));
    setPixmap(QPixmap(":/icons/backLiveLog128x128"));
    switchState(fsLiveLog);
    CLiveLogDB::self().gainFocus();
    funcMoveArea();
}

void CMegaMenu::funcDiary()
{
    CDiaryDB::self().openEditWidget();
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
    if(CCreateMapGeoTiff::self()){
        setEnabled(false);
        connect(CCreateMapGeoTiff::self(), SIGNAL(destroyed(QObject*)), this, SLOT(slotEnable()));
    }
}


void CMegaMenu::funcNewWpt()
{
    canvas->setMouseMode(CCanvas::eMouseAddWpt);
}


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

void CMegaMenu::funcDownloadTrack()
{
    CTrackDB::self().download();
}

void CMegaMenu::funcLiveLog()
{
    IDevice * dev = CResources::self().device();
    if(dev == 0) return;

    dev->setLiveLog(!dev->liveLog());
}
