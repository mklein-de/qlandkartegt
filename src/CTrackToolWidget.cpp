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
#include "CDlgTrackFilter.h"

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

    contextMenu     = new QMenu(this);
    actEdit         = contextMenu->addAction(QPixmap(":/icons/iconEdit16x16.png"),tr("Edit..."),this,SLOT(slotEdit()));
    actFilter       = contextMenu->addAction(QPixmap(":/icons/iconFilter16x16.png"),tr("Filter..."),this,SLOT(slotFilter()));
    actRevert       = contextMenu->addAction(QPixmap(":/icons/iconReload16x16.png"),tr("Revert"),this,SLOT(slotRevert()));
    actDistance     = contextMenu->addAction(QPixmap(":/icons/iconDistance16x16.png"),tr("Make Overlay"),this,SLOT(slotToOverlay()));
    actHide         = contextMenu->addAction(tr("Show"),this,SLOT(slotShow()));
    actShowBullets  = contextMenu->addAction(tr("Show Bullets"),this,SLOT(slotShowBullets()));
    actZoomToFit    = contextMenu->addAction(QPixmap(":/icons/iconZoomArea16x16.png"),tr("Zoom to fit"),this,SLOT(slotZoomToFit()));
    actDeSel        = contextMenu->addAction(QPixmap(":/icons/iconClear16x16.png"),tr("Deselect"),this,SLOT(slotDelSelect()));
    actDel          = contextMenu->addAction(QPixmap(":/icons/iconClear16x16.png"),tr("Delete"),this,SLOT(slotDelete()),Qt::Key_Delete);


    actHide->setCheckable(true);
    actShowBullets->setCheckable(true);
    actShowBullets->setChecked(CTrackDB::self().getShowBullets());

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
    while(track != tracks.end())
    {
        QListWidgetItem * item = new QListWidgetItem(listTracks);
        icon.fill((*track)->getColor());

        QPainter p;
        p.begin(&icon);

        if((*track)->isHidden())
        {
            p.drawPixmap(0,0,QPixmap(":icons/iconClear16x16"));
        }
        else
        {
            p.drawPixmap(0,0,QPixmap(":icons/iconOk16x16"));
        }
        p.end();

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
        if(days)
        {
            str += tr("\ntime: %1:").arg(days) + time.toString("HH:mm:ss");
        }
        else
        {
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

        if((*track)->isHighlighted())
        {
            highlighted = item;
        }

        ++track;
    }

    listTracks->sortItems();

    if(highlighted)
    {
        listTracks->setCurrentItem(highlighted);
    }
}


void CTrackToolWidget::slotItemDoubleClicked(QListWidgetItem * item)
{
    QString key = item->data(Qt::UserRole).toString();

    QRectF r = CTrackDB::self().getBoundingRectF(key);
    if (!r.isNull ())
    {
        CMapDB::self().getMap().zoom(r.left() * DEG_TO_RAD, r.top() * DEG_TO_RAD, r.right() * DEG_TO_RAD, r.bottom() * DEG_TO_RAD);
    }
}


void CTrackToolWidget::slotItemClicked(QListWidgetItem * item)
{
    originator = true;
    CTrackDB::self().highlightTrack(item->data(Qt::UserRole).toString());
    originator = false;
}


void CTrackToolWidget::keyPressEvent(QKeyEvent * e)
{
    if(e->key() == Qt::Key_Delete)
    {
        slotDelete();
        e->accept();
    }
    else
    {
        QWidget::keyPressEvent(e);
    }
}


void CTrackToolWidget::slotContextMenu(const QPoint& pos)
{
    int cnt = listTracks->selectedItems().count();
    if(cnt > 0)
    {
        if(listTracks->currentItem())
        {
            originator = true;
            CTrackDB::self().highlightTrack(listTracks->currentItem()->data(Qt::UserRole).toString());
            originator = false;
        }

        if(cnt > 1)
        {
            actEdit->setEnabled(false);
            actFilter->setEnabled(false);
            actRevert->setEnabled(false);
            actDeSel->setEnabled(false);
        }
        else
        {
            actEdit->setEnabled(true);
            actFilter->setEnabled(true);
            actRevert->setEnabled(true);
            actDeSel->setEnabled(true);
        }

        actHide->setChecked(!CTrackDB::self().highlightedTrack()->isHidden());

        QPoint p = listTracks->mapToGlobal(pos);
        contextMenu->exec(p);
    }

}


void CTrackToolWidget::slotEdit()
{
    const QListWidgetItem* item = listTracks->currentItem();

    if(item == 0)
    {
        QMessageBox::information(0,tr("Edit track ..."), tr("You have to select a track first."),QMessageBox::Ok,QMessageBox::Ok);
        return;
    };

    if(trackedit.isNull())
    {
        trackedit = new CTrackEditWidget(theMainWindow->getCanvas());
        connect(&CTrackDB::self(), SIGNAL(sigHighlightTrack(CTrack*)), trackedit, SLOT(slotSetTrack(CTrack*)));
        theMainWindow->setTempWidget(trackedit);
        trackedit->slotSetTrack(CTrackDB::self().highlightedTrack());
    }
    else
    {
        delete trackedit;
    }
}


void CTrackToolWidget::slotDelete()
{
    QStringList keys;
    QListWidgetItem * item;
    const QList<QListWidgetItem*>& items = listTracks->selectedItems();
    foreach(item,items)
    {
        keys << item->data(Qt::UserRole).toString();
        delete item;
    }
    CTrackDB::self().delTracks(keys);
}


void CTrackToolWidget::slotShow()
{
    QStringList keys;
    QListWidgetItem * item;
    const QList<QListWidgetItem*>& items = listTracks->selectedItems();
    foreach(item,items)
    {
        keys << item->data(Qt::UserRole).toString();
    }
    CTrackDB::self().hideTrack(keys, !actHide->isChecked());;
}


void CTrackToolWidget::slotDelSelect()
{
    const QListWidgetItem* item = listTracks->currentItem();
    if(item == 0)
    {
        return;
    }

    CTrackDB::self().highlightTrack("");
}


void CTrackToolWidget::slotToOverlay()
{
    CTrack * track;
    const QMap<QString,CTrack*>& tracks = CTrackDB::self().getTracks();

    QListWidgetItem * item;
    const QList<QListWidgetItem*>& items = listTracks->selectedItems();
    foreach(item,items)
    {
        track = tracks[item->data(Qt::UserRole).toString()];

        QList<XY> pts;

        CTrack::pt_t trkpt;
        QList<CTrack::pt_t>& trkpts = track->getTrackPoints();
        foreach(trkpt, trkpts)
        {
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


void CTrackToolWidget::slotFilter()
{
    const QListWidgetItem* item = listTracks->currentItem();

    if(item == 0)
    {
        QMessageBox::information(0,tr("Filter"), tr("You have to select a track first."),
            QMessageBox::Ok,QMessageBox::Ok);
        return;
    };

    CTrack *track = CTrackDB::self().highlightedTrack();
    CDlgTrackFilter dlg(*track, this);
    dlg.exec();
}


void CTrackToolWidget::slotZoomToFit()
{
    QRectF r;

    const QList<QListWidgetItem*>& items = listTracks->selectedItems();
    QList<QListWidgetItem*>::const_iterator item = items.begin();

    r = CTrackDB::self().getBoundingRectF((*item)->data(Qt::UserRole).toString());

    while(item != items.end())
    {
        r |= CTrackDB::self().getBoundingRectF((*item)->data(Qt::UserRole).toString());
        ++item;
    }

    if (!r.isNull ())
    {
        CMapDB::self().getMap().zoom(r.left() * DEG_TO_RAD, r.top() * DEG_TO_RAD, r.right() * DEG_TO_RAD, r.bottom() * DEG_TO_RAD);
    }
}


void CTrackToolWidget::slotRevert()
{
    const QListWidgetItem* item = listTracks->currentItem();

    if(item == 0)
    {
        QMessageBox::information(0,tr("Filter"), tr("You have to select a track first."), QMessageBox::Ok,QMessageBox::Ok);
        return;
    };

    CTrackDB::self().revertTrack(item->data(Qt::UserRole).toString());
}

void CTrackToolWidget::slotShowBullets()
{

    CTrackDB::self().setShowBullets(!CTrackDB::self().getShowBullets());

}
