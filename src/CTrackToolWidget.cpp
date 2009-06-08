/**********************************************************************************************
    Copyright (C) 2008 Oliver Eichler oliver.eichler@gmx.de

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

#include "CTrackToolWidget.h"
#include "CTrackEditWidget.h"
#include "CTrackDB.h"
#include "CTrack.h"
#include "CMapDB.h"
#include "CMainWindow.h"
#include "IUnit.h"
#include "COverlayDB.h"
#include "CMegaMenu.h"

#include <QtGui>

#define N_LINES 6

CTrackToolWidget::CTrackToolWidget(QTabWidget * parent)
: QWidget(parent)
, originator(false)
{
    setupUi(this);
    setObjectName("Tracks");
    parent->addTab(this,QIcon(":/icons/iconTrack16x16"),"");
    parent->setTabToolTip(parent->indexOf(this), tr("Tracks"));

    connect(&CTrackDB::self(), SIGNAL(sigChanged()), this, SLOT(slotDBChanged()));

    connect(listTracks,SIGNAL(itemClicked(QListWidgetItem*) ),this,SLOT(slotItemClicked(QListWidgetItem*)));
    connect(listTracks,SIGNAL(itemDoubleClicked(QListWidgetItem*) ),this,SLOT(slotItemDoubleClicked(QListWidgetItem*)));

    contextMenu = new QMenu(this);
    contextMenu->addAction(QPixmap(":/icons/iconEdit16x16.png"),tr("Edit..."),this,SLOT(slotEdit()));
    contextMenu->addAction(QPixmap(":/icons/iconDistance16x16.png"),tr("Make Overlay"),this,SLOT(slotToOverlay()));
    contextMenu->addAction(QPixmap(":/icons/iconClear16x16.png"),tr("Delete"),this,SLOT(slotDelete()),Qt::Key_Delete);

    connect(listTracks,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(slotContextMenu(const QPoint&)));

    QFontMetrics fm(listTracks->font());
    listTracks->setIconSize(QSize(15,N_LINES*fm.height()));
}


CTrackToolWidget::~CTrackToolWidget()
{

}


void CTrackToolWidget::slotDBChanged()
{
    if(originator) return;

    QFontMetrics fm(listTracks->font());
    QPixmap icon(15,N_LINES*fm.height());
    listTracks->clear();

    QListWidgetItem * highlighted = 0;

    const QMap<QString,CTrack*>& tracks = CTrackDB::self().getTracks();
    QMap<QString,CTrack*>::const_iterator track = tracks.begin();
    while(track != tracks.end()) {
        QListWidgetItem * item = new QListWidgetItem(listTracks);
        icon.fill((*track)->getColor());

        QString val1, unit1, val2, unit2;

        QString str     = (*track)->getName();
        double distance = (*track)->getTotalDistance();

        IUnit::self().meter2distance((*track)->getTotalDistance(), val1, unit1);
        str += tr("\nlength: %1 %2").arg(val1).arg(unit1);
        str += tr(", points: %1").arg((*track)->getTrackPoints().count());

        quint32 ttime = (*track)->getTotalTime();
        quint32 days  = ttime / 86400;

        QTime time;
        time = time.addSecs(ttime);
        if(days) {
            str += tr("\ntime: %1:").arg(days) + time.toString("HH:mm:ss");
        }
        else {
            str += tr("\ntime: ") + time.toString("HH:mm:ss");
        }

        IUnit::self().meter2speed(distance / ttime, val1, unit1);
        str += tr(", speed: %1 %2").arg(val1).arg(unit1);

        str += tr("\nstart: %1").arg((*track)->getStartTimestamp().isNull() ? tr("-") : (*track)->getStartTimestamp().toString());
        str += tr("\nend: %1").arg((*track)->getEndTimestamp().isNull() ? tr("-") : (*track)->getEndTimestamp().toString());

        IUnit::self().meter2elevation((*track)->getAscend(), val1, unit1);
        IUnit::self().meter2elevation((*track)->getDescend(), val2, unit2);

        str += tr("\n%1%2 %3, %4%5 %6").arg(QChar(0x2191)).arg(val1).arg(unit1).arg(QChar(0x2193)).arg(val2).arg(unit2);
        item->setText(str);
        item->setData(Qt::UserRole, (*track)->key());
        item->setIcon(icon);

        if((*track)->isHighlighted()) {
            highlighted = item;
        }

        ++track;
    }

    if(highlighted) {
        listTracks->setCurrentItem(highlighted);
    }
}


void CTrackToolWidget::slotItemDoubleClicked(QListWidgetItem * item)
{
    QString key = item->data(Qt::UserRole).toString();

    QRectF r = CTrackDB::self().getBoundingRectF(key);
    if (!r.isNull ())
        CMapDB::self().getMap().zoom(r.left() * DEG_TO_RAD, r.top() * DEG_TO_RAD, r.right() * DEG_TO_RAD, r.bottom() * DEG_TO_RAD);
}


void CTrackToolWidget::slotItemClicked(QListWidgetItem * item)
{
    originator = true;
    CTrackDB::self().highlightTrack(item->data(Qt::UserRole).toString());
    originator = false;
}


void CTrackToolWidget::keyPressEvent(QKeyEvent * e)
{
    if(e->key() == Qt::Key_Delete) {
        slotDelete();
        e->accept();
    }
    else {
        QWidget::keyPressEvent(e);
    }
}


void CTrackToolWidget::slotContextMenu(const QPoint& pos)
{
    if(listTracks->currentItem()) {
        QPoint p = listTracks->mapToGlobal(pos);
        contextMenu->exec(p);
    }

}


void CTrackToolWidget::slotEdit()
{
    const QListWidgetItem* item = listTracks->currentItem();

    if(item == 0) {
        QMessageBox::information(0,tr("Edit track ..."), tr("You have to select a track first."),QMessageBox::Ok,QMessageBox::Ok);
        return;
    };

    if(trackedit.isNull()) {
        trackedit = new CTrackEditWidget(theMainWindow->getCanvas());
        connect(&CTrackDB::self(), SIGNAL(sigHighlightTrack(CTrack*)), trackedit, SLOT(slotSetTrack(CTrack*)));
        theMainWindow->setTempWidget(trackedit);
        trackedit->slotSetTrack(CTrackDB::self().highlightedTrack());
    }
    else {
        delete trackedit;
    }
}


void CTrackToolWidget::slotDelete()
{
    QStringList keys;
    QListWidgetItem * item;
    const QList<QListWidgetItem*>& items = listTracks->selectedItems();
    foreach(item,items) {
        keys << item->data(Qt::UserRole).toString();
        delete item;
    }
    CTrackDB::self().delTracks(keys);
}


void CTrackToolWidget::slotToOverlay()
{
    CTrack * track;
    const QMap<QString,CTrack*>& tracks = CTrackDB::self().getTracks();

    QListWidgetItem * item;
    const QList<QListWidgetItem*>& items = listTracks->selectedItems();
    foreach(item,items) {
        track = tracks[item->data(Qt::UserRole).toString()];

        QList<XY> pts;

        CTrack::pt_t trkpt;
        QList<CTrack::pt_t>& trkpts = track->getTrackPoints();
        foreach(trkpt, trkpts) {
            if(trkpt.flags & CTrack::pt_t::eDeleted) continue;

            XY pt;
            pt.u = trkpt.lon * DEG_TO_RAD;
            pt.v = trkpt.lat * DEG_TO_RAD;

            pts << pt;
        }

        COverlayDB::self().addDistance(track->name, tr("created from track"), pts);
    }

    CMegaMenu::self().switchByKeyWord("Overlay");
}
