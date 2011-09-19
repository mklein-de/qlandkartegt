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

#include "CTrackEditWidget.h"
#include "CTrackStatProfileWidget.h"
#include "CTrackStatSpeedWidget.h"
#include "CTrackStatDistanceWidget.h"
#include "CTrackStatTraineeWidget.h"
                                 //Anfgen des Ext. Widgets
#ifdef GPX_EXTENSIONS
#include "CTrackStatExtensionWidget.h"
#endif
#include "CTrack.h"
#include "CTrackDB.h"
#include "CResources.h"
#include "GeoMath.h"
#include "CMainWindow.h"
#include "CTabWidget.h"
#include "IUnit.h"
#include "CMenus.h"
#include "CActions.h"
#include "CDlgTrackFilter.h"
#include "CWptDB.h"


#include <QtGui>

bool CTrackTreeWidgetItem::operator< ( const QTreeWidgetItem & other ) const
{
    const QString speed("/h");
    const QRegExp distance("(ft|ml|m|km)");
    double d1 = 0, d2 = 0;

    int sortCol = treeWidget()->sortColumn();
    QString str1 = text(sortCol);
    QString str2 = other.text(sortCol);

    if (str1.contains(speed) && str2.contains(speed))
    {
        d1 = IUnit::self().str2speed(str1);
        d2 = IUnit::self().str2speed(str2);
    }
    else if (str1.contains(distance) && str2.contains(distance))
    {
        d1 = IUnit::self().str2distance(str1);
        d2 = IUnit::self().str2distance(str2);
    }
    else
    {
        /* let's assume it's a double without any unit ... */
        d1 = str1.toDouble();
        d2 = str2.toDouble();
    }

    return d1 < d2;
}


CTrackEditWidget::CTrackEditWidget(QWidget * parent)
: QWidget(parent)
, originator(false)
#ifdef GPX_EXTENSIONS
, Vspace(0)
, tabstat(0)
, no_ext_info_stat(0)
#endif
{
    setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose,true);

#ifndef GPX_EXTENSIONS
    tabWidget->removeTab(1);
#endif

    toolGraphDistance->setIcon(QIcon(":/icons/iconGraph16x16.png"));
    connect(toolGraphDistance, SIGNAL(clicked()), this, SLOT(slotToggleStatDistance()));

    toolGraphTime->setIcon(QIcon(":/icons/iconTime16x16.png"));
    connect(toolGraphTime, SIGNAL(clicked()), this, SLOT(slotToggleStatTime()));

    traineeGraph->setIcon(QIcon(":/icons/package_favorite.png"));
    connect(traineeGraph, SIGNAL(clicked()), this, SLOT(slotToggleTrainee()));

    toolFilter->setIcon(QIcon(":/icons/iconFilter16x16.png"));
    connect(toolFilter, SIGNAL(clicked()), this, SLOT(slotFilter()));

    toolReset->setIcon(QIcon(":/icons/editundo.png"));
    connect(toolReset, SIGNAL(clicked()), this, SLOT(slotReset()));

    toolDelete->setIcon(QIcon(":/icons/iconDelete16x16.png"));
    connect(toolDelete, SIGNAL(clicked()), this, SLOT(slotDelete()));

    QPixmap icon(16,8);
    for(int i=0; i < 17; ++i)
    {
        icon.fill(CTrack::lineColors[i]);
        comboColor->addItem(icon,"",QVariant(i));
    }

    connect(treePoints,SIGNAL(itemSelectionChanged()),this,SLOT(slotPointSelectionChanged()));
    connect(treePoints,SIGNAL(itemClicked(QTreeWidgetItem*,int)),this,SLOT(slotPointSelection(QTreeWidgetItem*)));

#ifdef GPX_EXTENSIONS
    //------------------------------------
    //add icon for extension & connect
    toolGraphExtensions->setIcon(QIcon(":/icons/iconExtensions16x16.png"));
    connect(toolGraphExtensions, SIGNAL(clicked()), this, SLOT(slotToggleExtensionsGraph()));

    //add icon for google maps
    toolGoogleMaps->setIcon(QIcon(":/icons/iconGoogleMaps16x16.png"));
    connect(toolGoogleMaps, SIGNAL(clicked()), this, SLOT(slotGoogleMaps()));

    //checkboxes to switch on/off standard columns
    connect(checkBox_num,SIGNAL(clicked(bool)),this,SLOT(slotSetColumns(bool)));
    connect(checkBox_tim,SIGNAL(clicked(bool)),this,SLOT(slotSetColumns(bool)));
    connect(checkBox_hig,SIGNAL(clicked(bool)),this,SLOT(slotSetColumns(bool)));
    connect(checkBox_dis,SIGNAL(clicked(bool)),this,SLOT(slotSetColumns(bool)));
    connect(checkBox_azi,SIGNAL(clicked(bool)),this,SLOT(slotSetColumns(bool)));
    connect(checkBox_ent,SIGNAL(clicked(bool)),this,SLOT(slotSetColumns(bool)));
    connect(checkBox_vel,SIGNAL(clicked(bool)),this,SLOT(slotSetColumns(bool)));
    connect(checkBox_suu,SIGNAL(clicked(bool)),this,SLOT(slotSetColumns(bool)));
    connect(checkBox_sud,SIGNAL(clicked(bool)),this,SLOT(slotSetColumns(bool)));
    connect(checkBox_pos,SIGNAL(clicked(bool)),this,SLOT(slotSetColumns(bool)));

#else
    toolGraphExtensions->hide();
    toolGoogleMaps->hide();
#endif

    treePoints->sortByColumn(eNum, Qt::AscendingOrder);

    CActions * actions = theMainWindow->getActionGroupProvider()->getActions();

    contextMenu = new QMenu(this);
    contextMenu->addAction(actions->getAction("aTrackPurgeSelection"));
    actSplit    = contextMenu->addAction(QPixmap(":/icons/iconEditCut16x16.png"),tr("Split"),this,SLOT(slotSplit()));
    connect(treePoints,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(slotContextMenu(const QPoint&)));

    connect(comboColor, SIGNAL(currentIndexChanged(int)), this, SLOT(slotColorChanged(int)));
    connect(lineName, SIGNAL(returnPressed()), this, SLOT(slotNameChanged()));
    connect(lineName, SIGNAL(textChanged(QString)), this, SLOT(slotNameChanged(QString)));

    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(slotCurrentChanged(int)));

    connect(&CWptDB::self(), SIGNAL(sigChanged()), this, SLOT(slotWptChanged()));
    connect(&CTrackDB::self(), SIGNAL(sigChanged()), this, SLOT(slotWptChanged()));
}


CTrackEditWidget::~CTrackEditWidget()
{
    if(!trackStatProfileDist.isNull())
    {
        delete trackStatProfileDist;
    }
    if(!trackStatSpeedDist.isNull())
    {
        delete trackStatSpeedDist;
    }
    if(!trackStatProfileTime.isNull())
    {
        delete trackStatProfileTime;
    }
    if(!trackStatSpeedTime.isNull())
    {
        delete trackStatSpeedTime;
    }
    if(!trackStatDistanceTime.isNull())
    {
        delete trackStatDistanceTime;
    }
    if(!trackStatTrainee.isNull())
    {
        delete trackStatTrainee;
    }
#ifdef GPX_EXTENSIONS
    //delete all extension tabs and reset tab status
    int num_of_tabs  = trackStatExtensions.size();
    for(int i=0; i < num_of_tabs; ++i)
    {
        if (trackStatExtensions[i])
        {
            delete trackStatExtensions[i];
        }
    }
    tabstat = 0;
    trackStatExtensions.clear();
#endif

}


void CTrackEditWidget::keyPressEvent(QKeyEvent * e)
{
    if(track.isNull()) return;

    if(e->key() == Qt::Key_Delete)
    {
        slotPurge();
    }
    else
    {
        QWidget::keyPressEvent(e);
    }
}

void CTrackEditWidget::resizeEvent(QResizeEvent * e)
{
    QWidget::resizeEvent(e);

    slotWptChanged();
}

void CTrackEditWidget::slotContextMenu(const QPoint& pos)
{
    int cnt = treePoints->selectedItems().count();
    if(cnt > 0)
    {

        actSplit->setEnabled(cnt == 1);


        QPoint p = treePoints->mapToGlobal(pos);
        contextMenu->exec(p);
    }

}


void CTrackEditWidget::slotSplit()
{
    QList<QTreeWidgetItem *> items = treePoints->selectedItems();

    if(items.isEmpty())
    {
        return;
    }

    int idx = items.first()->text(eNum).toInt();

    CTrackDB::self().splitTrack(idx);
}


void CTrackEditWidget::slotSetTrack(CTrack * t)
{
    if(originator) return;

    treePoints->clear();

    if(track)
    {
        disconnect(track,SIGNAL(sigChanged()), this, SLOT(slotUpdate()));
        disconnect(track,SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));


#ifdef GPX_EXTENSIONS
        //delete all extension tabs and reset tab status
        int num_of_tabs  = trackStatExtensions.size();
        for(int i=0; i < num_of_tabs; ++i)
        {
            if (trackStatExtensions[i])
            {
                delete trackStatExtensions[i];
            }
        }
        tabstat = 0;
        trackStatExtensions.clear();
        toolGraphExtensions->setChecked(false);

        //delete extensions columns in table
        //get names and number of extensions
        QList<QString> names_of_ext = track->tr_ext.set.toList();                                 //Anzahl der Extensions
        int num_of_ext              = names_of_ext.size();
        for(int i=0; i < num_of_ext; ++i)
        {
            int number_of_column = eMaxColumn + i;
            //remove extensions columns - removeItemWidget is not sufficient ...
            treePoints->hideColumn(number_of_column);
            treePoints->removeItemWidget(treePoints->headerItem(), number_of_column);
        }

        //delete checkboxes & spacer
        for(int i=0; i < c_boxes.size(); ++i)
        {
            delete c_boxes[i];   //remove checkboxes
        }
        c_boxes.clear();         //empty qlist

        //remove spacer
        verticalLayout_Extensions->removeItem(Vspace);
        gridLayout_Extensions->removeWidget(label);

        //------------------------------------------------------------------------------------
#endif
    }

    track = t;
    if(track.isNull())
    {
        close();
        return;
    }

    //----------------------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------

    //for each extension: create a checkbox and a column and link them together
#ifdef GPX_EXTENSIONS

    //get names and number of extensions
    QList<QString> names_of_ext = track->tr_ext.set.toList();                                 //Anzahl der Extensions
    int num_of_ext              = names_of_ext.size();

    if (num_of_ext)
    {
        gridLayout_Extensions->removeWidget(label);

        //create checkboxes
        for(int i=0; i < num_of_ext; ++i)
        {
            //Namen der Extentions pro i in Variable
            QString name        = names_of_ext[i];
            //column number
            int number_of_column = eMaxColumn + i;
            //Name des objects
            QString obj_name    = QString("%1").arg(number_of_column);

            //Check box generieren, checken und mit Name versehen
            QCheckBox *CheckBoxMake;
            CheckBoxMake = new QCheckBox(name);
            CheckBoxMake->setChecked(true);
            CheckBoxMake->setObjectName(obj_name);
            //CheckBoxMake->setToolTip(obj_name);
            c_boxes.insert(i, CheckBoxMake);
            verticalLayout_Extensions->addWidget(CheckBoxMake, i, 0);

            //add columns
            treePoints->headerItem()->setText(number_of_column, name);
            treePoints->showColumn(number_of_column);

            //connect checkbox with column
            connect(CheckBoxMake ,SIGNAL(clicked(bool)),this,SLOT(slotSetColumnsExt(bool)));
        }

        //QSpacerItem *Vspace;
        Vspace = new QSpacerItem(20, 2, QSizePolicy::Minimum, QSizePolicy::Expanding);
        verticalLayout_Extensions->addItem(Vspace);

    }
    else
    {
        if (no_ext_info_stat == 0)
        {
            QString lname = "no_ext_info";
            label = new QLabel(lname);
            label->setObjectName(lname);
            label->setEnabled(false);
            QFont font;
            font.setBold(true);
            font.setItalic(false);
            font.setWeight(75);
            font.setStyleStrategy(QFont::PreferDefault);
            label->setFont(font);
            label->setAlignment(Qt::AlignCenter);
             //keine extensions Elemente in dieser Datei
            label->setText(tr("no extensions elements in this file"));
            verticalLayout_Extensions->addWidget(label);
            no_ext_info_stat = 1;
        }
        else
        {
            verticalLayout_Extensions->removeWidget(label);
            no_ext_info_stat = 0;
        }
    }

    //----------------------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------
#endif

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QList<CTrack::pt_t>& trkpts           = track->getTrackPoints();
    QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();
    while(trkpt != trkpts.end())
    {
        trkpt->editItem = 0;
        ++trkpt;
    }

    connect(track,SIGNAL(sigChanged()), this, SLOT(slotUpdate()));
    connect(track,SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));

    slotUpdate();

    //TODO: resize of the TrackEditWidget
    QSettings cfg;
    // restore last session position and size of TrackEditWidget
    if ( cfg.contains("TrackEditWidget/geometry"))
    {
        QRect r = cfg.value("TrackEditWidget/geometry").toRect();

        if (r.isValid() && QDesktopWidget().screenGeometry().intersects(r))
            {tabWidget->setGeometry(r);}
    }
    else
    {
        cfg.setValue("TrackEditWidget/geometry",tabWidget->geometry());
    }

    treePoints->setUpdatesEnabled(false);

#ifdef GPX_EXTENSIONS
    for(int i=0; i < eMaxColumn+num_of_ext-1; ++i)
#else
    for(int i=0; i < eMaxColumn; ++i)
#endif
    {
        treePoints->resizeColumnToContents(i);
    }
    treePoints->setUpdatesEnabled(true);

    QApplication::restoreOverrideCursor();

    slotWptChanged();
}



void CTrackEditWidget::slotUpdate()
{
    int i;

    if (track->hasTraineeData())
        traineeGraph->setEnabled(true);
    else
    {
        traineeGraph->setEnabled(false);
        if (!trackStatTrainee.isNull())
            delete trackStatTrainee;
    }

#ifdef GPX_EXTENSIONS
    //TODO: endable ext. sym. only when there are exts

    //get names and number of extensions
    QList<QString> names_of_ext = track->tr_ext.set.toList();                                 //Anzahl der Extensions
    int num_of_ext              = names_of_ext.size();

    if (num_of_ext == 0)
    {
        toolGraphExtensions->setEnabled(false);
    }
    else
    {
        toolGraphExtensions->setEnabled(true);
    }
#endif

    if(originator) return;

    lineName->setText(track->getName());
    comboColor->setCurrentIndex(track->getColorIdx());

    treePoints->setUpdatesEnabled(false);
    treePoints->setSelectionMode(QAbstractItemView::MultiSelection);

    QString str, val, unit;
    CTrackTreeWidgetItem * focus    = 0;
    QList<CTrack::pt_t>& trkpts           = track->getTrackPoints();
    QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();

    while(trkpt != trkpts.end())
    {
        CTrackTreeWidgetItem * item;
        if ( !trkpt->editItem )
        {
            item = new CTrackTreeWidgetItem(treePoints);
            item->setTextAlignment(eNum,Qt::AlignLeft);
            item->setTextAlignment(eAltitude,Qt::AlignRight);
            item->setTextAlignment(eDelta,Qt::AlignRight);
            item->setTextAlignment(eAzimuth,Qt::AlignRight);
            item->setTextAlignment(eDistance,Qt::AlignRight);
            item->setTextAlignment(eAscend,Qt::AlignRight);
            item->setTextAlignment(eDescend,Qt::AlignRight);
            item->setTextAlignment(eSpeed,Qt::AlignRight);

            trkpt->editItem = item;
            trkpt->flags.setChanged(true);
        }

        if ( !trkpt->flags.isChanged() )
        {
            ++trkpt;
            continue;
        }

        item = (CTrackTreeWidgetItem *)trkpt->editItem.data();
        item->setData(0, Qt::UserRole, trkpt->idx);

        // gray shade deleted items
        if(trkpt->flags & CTrack::pt_t::eDeleted)
        {
            //item->setFlags((item->flags() & ~Qt::ItemIsEnabled) | Qt::ItemIsTristate);
#ifdef GPX_EXTENSIONS
            for(i = 0; i < eMaxColumn+num_of_ext; ++i)
#else
            for(i = 0; i < eMaxColumn; ++i)
#endif
            {
                item->setForeground(i,QBrush(Qt::gray));
            }
        }
        else
        {
            //item->setFlags(item->flags() | Qt::ItemIsEnabled | Qt::ItemIsTristate);
#ifdef GPX_EXTENSIONS
            for(i = 0; i < eMaxColumn+num_of_ext; ++i)
#else
            for(i = 0; i < eMaxColumn; ++i)
#endif
            {
                item->setForeground(i,QBrush(Qt::black));
            }
        }

        // temp. store item of user focus
        if(trkpt->flags & CTrack::pt_t::eFocus)
        {
            focus = item;
        }

        if(trkpt->flags & CTrack::pt_t::eSelected)
        {
            if ( !item->isSelected() )
                item->setSelected(true);
        }
        else
        {
            if ( item->isSelected() )
                item->setSelected(false);
        }

        // point number
        item->setText(eNum,QString::number(trkpt->idx));

        // timestamp
        if(trkpt->timestamp != 0x00000000 && trkpt->timestamp != 0xFFFFFFFF)
        {
            QDateTime time = QDateTime::fromTime_t(trkpt->timestamp);
            time.setTimeSpec(Qt::LocalTime);
            str = time.toString();
        }
        else
        {
            str = "-";
        }

        item->setText(eTime,str);

        // altitude
        if(trkpt->ele != WPT_NOFLOAT)
        {
            IUnit::self().meter2elevation(trkpt->ele, val, unit);
            str = tr("%1 %2").arg(val).arg(unit);
        }
        else
        {
            str = "-";
        }
        item->setText(eAltitude,str);

        // delta
        IUnit::self().meter2distance(trkpt->delta, val, unit);
        item->setText(eDelta, tr("%1 %2").arg(val).arg(unit));

        // azimuth
        if(trkpt->azimuth != WPT_NOFLOAT)
        {
            str.sprintf("%1.0f\260",trkpt->azimuth);
        }
        else
        {
            str = "-";
        }
        item->setText(eAzimuth,str);

        // distance
        IUnit::self().meter2distance(trkpt->distance, val, unit);
        item->setText(eDistance, tr("%1 %2").arg(val).arg(unit));
        IUnit::self().meter2elevation(trkpt->ascend, val, unit);
        item->setText(eAscend, tr("%1 %2").arg(val).arg(unit));
        IUnit::self().meter2elevation(trkpt->descend, val, unit);
        item->setText(eDescend, tr("%1 %2").arg(val).arg(unit));

        // speed
        if(trkpt->speed > 0)
        {
            IUnit::self().meter2speed(trkpt->speed, val, unit);
            str = tr("%1 %2").arg(val).arg(unit);
        }
        else
        {
            str = "-";
        }
        item->setText(eSpeed,str);


        // position
        GPS_Math_Deg_To_Str(trkpt->lon, trkpt->lat, str);
        item->setText(ePosition,str);


#ifdef GPX_EXTENSIONS
        //fill cells of tracklist with extensions
        for(int i=0; i < num_of_ext; ++i)
        {
            int col = eMaxColumn+i;

            //name of the extension
            QString nam = names_of_ext[i];

            //value of the extension
            QString val = trkpt->gpx_exts.getValue(nam);

            if (val == "")
            {
                val = "-";;
            }

            //insert
            item->setText(col, val);
            //format
            item->setTextAlignment(col, Qt::AlignRight);

        }
#endif
        //--------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------------------------------------------

        trkpt->flags.setChanged(false);

        ++trkpt;
    }

    // adjust column sizes to fit
    treePoints->header()->setResizeMode(0,QHeaderView::Interactive);

    // scroll to item of user focus
    if(focus)
    {
        //treePoints->setCurrentItem(focus);
        treePoints->scrollToItem(focus);
    }
    treePoints->setSelectionMode(QAbstractItemView::ExtendedSelection);
    treePoints->setUpdatesEnabled(true);
}


void CTrackEditWidget::slotPointSelectionChanged()
{
    if(track.isNull() || originator) return;

    if(treePoints->selectionMode() == QAbstractItemView::MultiSelection) return;

    //    qDebug() << Q_FUNC_INFO;

    // reset previous selections
    QList<CTrack::pt_t>& trkpts           = track->getTrackPoints();
    QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();
    while(trkpt != trkpts.end())
    {
        trkpt->flags &= ~CTrack::pt_t::eSelected;
        ++trkpt;
    }

    // set eSelected flag for selected points
    QList<QTreeWidgetItem*> items = treePoints->selectedItems();
    QList<QTreeWidgetItem*>::const_iterator item = items.begin();
    while(item != items.end())
    {
        quint32 idxTrkPt = (*item)->data(0,Qt::UserRole).toUInt();
        trkpts[idxTrkPt].flags |= CTrack::pt_t::eSelected;
        ++item;
    }
    originator = true;
    track->rebuild(false);
    originator = false;
}


void CTrackEditWidget::slotPointSelection(QTreeWidgetItem * item)
{
    if(track.isNull()) return;

    originator = true;
    track->setPointOfFocus(item->data(0,Qt::UserRole).toInt(), false, true);
    originator = false;
}


void CTrackEditWidget::slotPurge()
{
    QList<CTrack::pt_t>& trkpts                     = track->getTrackPoints();
    QList<QTreeWidgetItem*> items                   = treePoints->selectedItems();
    QList<QTreeWidgetItem*>::const_iterator item    = items.begin();

    while(item != items.end())
    {
        quint32 idxTrkPt = (*item)->data(0,Qt::UserRole).toUInt();
        if(trkpts[idxTrkPt].flags & CTrack::pt_t::eDeleted)
        {
            trkpts[idxTrkPt].flags &= ~CTrack::pt_t::eDeleted;
        }
        else
        {
            trkpts[idxTrkPt].flags |= CTrack::pt_t::eDeleted;
        }

        ++item;
    }
    track->rebuild(false);
    emit CTrackDB::self().sigModified();
    emit CTrackDB::self().sigModified(track->getKey());
}


void CTrackEditWidget::slotShowProfile()
{
    if(trackStatProfileDist.isNull())
    {
        if(trackStatSpeedDist.isNull())
        {
            trackStatSpeedDist = new CTrackStatSpeedWidget(ITrackStat::eOverDistance, this);
            theMainWindow->getCanvasTab()->setTabPosition(QTabWidget::South);
            theMainWindow->getCanvasTab()->addTab(trackStatSpeedDist, tr("Speed/Dist."));
        }

        if(trackStatDistanceTime.isNull())
        {
            trackStatDistanceTime = new CTrackStatDistanceWidget(this);
            theMainWindow->getCanvasTab()->setTabPosition(QTabWidget::South);
            theMainWindow->getCanvasTab()->addTab(trackStatDistanceTime, tr("Dist./Time"));
        }

        trackStatProfileDist = new CTrackStatProfileWidget(ITrackStat::eOverDistance, this);
        theMainWindow->getCanvasTab()->setTabPosition(QTabWidget::South);
        theMainWindow->getCanvasTab()->addTab(trackStatProfileDist, tr("Profile/Dist."));
        toolGraphDistance->toggle();
    }

    theMainWindow->getCanvasTab()->setCurrentWidget(trackStatProfileDist);
}

void CTrackEditWidget::slotToggleStatDistance()
{


    if(trackStatSpeedDist.isNull())
    {
        trackStatSpeedDist = new CTrackStatSpeedWidget(ITrackStat::eOverDistance, this);
        theMainWindow->getCanvasTab()->setTabPosition(QTabWidget::South);
        theMainWindow->getCanvasTab()->addTab(trackStatSpeedDist, tr("Speed/Dist."));
    }
    else
    {
        delete trackStatSpeedDist;
    }

    if(trackStatDistanceTime.isNull())
    {
        trackStatDistanceTime = new CTrackStatDistanceWidget(this);
        theMainWindow->getCanvasTab()->setTabPosition(QTabWidget::South);
        theMainWindow->getCanvasTab()->addTab(trackStatDistanceTime, tr("Dist./Time"));
    }
    else
    {
        delete trackStatDistanceTime;
    }

    if(trackStatProfileDist.isNull())
    {
        trackStatProfileDist = new CTrackStatProfileWidget(ITrackStat::eOverDistance, this);
        theMainWindow->getCanvasTab()->setTabPosition(QTabWidget::South);
        theMainWindow->getCanvasTab()->addTab(trackStatProfileDist, tr("Profile/Dist."));
    }
    else
    {
        delete trackStatProfileDist;
    }
}


void CTrackEditWidget::slotToggleStatTime()
{
    if(trackStatSpeedTime.isNull())
    {
        trackStatSpeedTime = new CTrackStatSpeedWidget(ITrackStat::eOverTime, this);
        theMainWindow->getCanvasTab()->setTabPosition(QTabWidget::South);
        theMainWindow->getCanvasTab()->addTab(trackStatSpeedTime, tr("Speed/Time"));
    }
    else
    {
        delete trackStatSpeedTime;
    }

    if(trackStatProfileTime.isNull())
    {
        trackStatProfileTime = new CTrackStatProfileWidget(ITrackStat::eOverTime, this);
        theMainWindow->getCanvasTab()->setTabPosition(QTabWidget::South);
                                 //Tab hinzufgen
        theMainWindow->getCanvasTab()->addTab(trackStatProfileTime, tr("Profile/Time"));
    }
    else
    {
        delete trackStatProfileTime;
    }
}


void CTrackEditWidget::slotToggleTrainee()
{
    if(trackStatTrainee.isNull())
    {
        trackStatTrainee = new CTrackStatTraineeWidget(ITrackStat::eOverDistance, this);
        theMainWindow->getCanvasTab()->setTabPosition(QTabWidget::South);
        theMainWindow->getCanvasTab()->addTab(trackStatTrainee, tr("Trainee"));
    }
    else
    {
        delete trackStatTrainee;
    }
}


#ifdef GPX_EXTENSIONS
//method to show & hide the extensions graphs
void CTrackEditWidget::slotToggleExtensionsGraph()
{
    //get names und number of extensions
    QList<QString> names_of_ext = track->tr_ext.set.toList();
    int num_of_ext              = names_of_ext.size();

    if (tabstat == 0)
    {
        for(int i=0; i < num_of_ext; ++i)
        {
            QString name = names_of_ext[i];
            QPointer<CTrackStatExtensionWidget> tab;

            //Create tab containing plot over time
            tab = new CTrackStatExtensionWidget(ITrackStat::eOverTime, this, name);

            //add tab
            theMainWindow->getCanvasTab()->addTab(tab, name+"/t");
            trackStatExtensions.insert(i, tab); //add Tab index to list for further handling
        }
        connect(theMainWindow->getCanvasTab(),SIGNAL(tabCloseRequested(int)),this,SLOT(slotKillTab(int)));
        tabstat = 1;
    }
    else
    {
        //delete all extension tabs and reset tab status
        int num_of_tabs  = trackStatExtensions.size();
        for(int i=0; i < num_of_tabs; ++i)
        {
            if (trackStatExtensions[i])
            {
                delete trackStatExtensions[i];
            }
        }
        disconnect(theMainWindow->getCanvasTab(),SIGNAL(tabCloseRequested(int)), this, SLOT(slotKillTab(int)));
        tabstat = 0;
        trackStatExtensions.clear();
    }


    //Tab Settings
    theMainWindow->getCanvasTab()->setTabPosition(QTabWidget::South);
    theMainWindow->getCanvasTab()->setMovable(true);

    //TODO: make tabs closeable
    theMainWindow->getCanvasTab()->setTabsClosable(true);
}
#endif

#ifdef GPX_EXTENSIONS
//TODO: method to switch on/off standard columns in track view
void CTrackEditWidget::slotSetColumns(bool checked)
{
    //who made the signal
    QString name_std_is = (QObject::sender()->objectName());

    //0. checkBox_num - number
    if (name_std_is == "checkBox_num")
    {
        if(checked == true) {CTrackEditWidget::treePoints->showColumn(eNum);}
        else                {CTrackEditWidget::treePoints->hideColumn(eNum);}
    }
    //1. checkBox_tim - time
    else if (name_std_is == "checkBox_tim")
    {
        if(checked == true) {CTrackEditWidget::treePoints->showColumn(eTime);}
        else                {CTrackEditWidget::treePoints->hideColumn(eTime);}
    }
    //2 .checkBox_hig - hight
    else if (name_std_is == "checkBox_hig")
    {
        if(checked == true) {CTrackEditWidget::treePoints->showColumn(eAltitude);}
        else                {CTrackEditWidget::treePoints->hideColumn(eAltitude);}
    }
    //3 .checkBox_dis - distance
    else if (name_std_is == "checkBox_dis")
    {
        if(checked == true) {CTrackEditWidget::treePoints->showColumn(eDelta);}
        else                {CTrackEditWidget::treePoints->hideColumn(eDelta);}
    }
    //4 .checkBox_azi - azimuth
    else if (name_std_is == "checkBox_azi")
    {
        if(checked == true) {CTrackEditWidget::treePoints->showColumn(eAzimuth);}
        else                {CTrackEditWidget::treePoints->hideColumn(eAzimuth);}
    }
    //5 .checkBox_ent - entfernung
    else if (name_std_is == "checkBox_ent")
    {
        if(checked == true) {CTrackEditWidget::treePoints->showColumn(eDistance);}
        else                {CTrackEditWidget::treePoints->hideColumn(eDistance);}
    }
    //6 .checkBox_vel - velocity
    else if (name_std_is == "checkBox_vel")
    {
        if(checked == true) {CTrackEditWidget::treePoints->showColumn(eSpeed);}
        else                {CTrackEditWidget::treePoints->hideColumn(eSpeed);}
    }
    //7 .checkBox_suu - summ up
    else if (name_std_is == "checkBox_suu")
    {
        if(checked == true) {CTrackEditWidget::treePoints->showColumn(eAscend);}
        else                {CTrackEditWidget::treePoints->hideColumn(eAscend);}
    }
    //8 .checkBox_sud - summ down
    else if (name_std_is == "checkBox_sud")
    {
        if(checked == true) {CTrackEditWidget::treePoints->showColumn(eDescend);}
        else                {CTrackEditWidget::treePoints->hideColumn(eDescend);}
    }
    //9 .checkBox_pos - position
    else if (name_std_is == "checkBox_pos")
    {
        if(checked == true) {CTrackEditWidget::treePoints->showColumn(ePosition);}
        else                {CTrackEditWidget::treePoints->hideColumn(ePosition);}
    }
    //sender unknown -> nothing happens
    else
    {
    }
}
#endif

#ifdef GPX_EXTENSIONS
//TODO: switch extension columns on/off
void CTrackEditWidget::slotSetColumnsExt(bool checked)
{
    //who's sender of checkbox signal
    QString nameis = (QObject::sender()->objectName());
    int col = nameis.toInt();    //use object name as column #

    //on or off
    if(checked == true) {CTrackEditWidget::treePoints->showColumn(col);}
    else                {CTrackEditWidget::treePoints->hideColumn(col);}

}
#endif


//TODO: Show Track in Google Maps
void CTrackEditWidget::slotGoogleMaps()
{
    QString str, outp;
    int count = 0;
    int pnts = 0;
    int every_this = 0;

    QList<CTrack::pt_t>& trkpts           = track->getTrackPoints();
    QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();

    pnts = trkpts.size();
    if (pnts <= 25)
    {

        while(trkpt != trkpts.end())
        {

            GPS_Math_Deg_To_Str(trkpt->lon, trkpt->lat, str);
            if (count == 0)     {outp += str;}
            else                {outp += "+to:"+str;}
            trkpt++;
            count++;
        }
    }
    else
    {

        every_this = (pnts/25);
        int icount = 0;
        int multi = 0;

        while(trkpt != trkpts.end())
        {
            if (count == every_this)
            {

                GPS_Math_Deg_To_Str(trkpt->lon, trkpt->lat, str);
                if (icount == 0)        {outp += str;}
                else                {outp += "+to:"+str;}
                //every_this += every_this;
                icount++;
                multi = icount+1;
            }
            else if (count == every_this*multi)
            {
                GPS_Math_Deg_To_Str(trkpt->lon, trkpt->lat, str);
                if (icount == 0)        {outp += str;}
                else                {outp += "+to:"+str;}
                //every_this += every_this;
                icount++;
                multi = icount+1;
            }

            //trkpt->idx = trkpt->idx+every_this;
            trkpt++;
            count++;
        }

    }
    /*
    QMessageBox msgBox;
    msgBox.setText("text");
    msgBox.exec();
    */
    QDesktopServices::openUrl(QUrl("http://maps.google.com/maps?f=h&saddr=&daddr="+outp, QUrl::TolerantMode));
}


//TODO: Close Tab
void CTrackEditWidget::slotKillTab(int index)
{
    if (index != 0)
    {
        theMainWindow->getCanvasTab()->removeTab(index);
    }
}

void CTrackEditWidget::slotColorChanged(int idx)
{
    if(track.isNull()) return;

    int _idx_ = track->getColorIdx();
    if(_idx_ != comboColor->currentIndex())
    {
        track->setColor(comboColor->currentIndex());
        track->rebuild(true);
        emit CTrackDB::self().sigModified();
        emit CTrackDB::self().sigModified(track->getKey());
    }
}

void CTrackEditWidget::slotNameChanged(const QString& name)
{
    if(track.isNull()) return;
    QString _name_ = track->getName();
    QPalette palette = lineName->palette();
    if(_name_ != name)
    {
        palette.setColor(QPalette::Base, QColor(255, 128, 128));
    }
    else
    {
        palette.setColor(QPalette::Base, QColor(255, 255, 255));
    }
    lineName->setPalette(palette);
}

void CTrackEditWidget::slotNameChanged()
{
    if(track.isNull()) return;

    QString  name  = lineName->text();
    QString _name_ = track->getName();

    QPalette palette = lineName->palette();

    if(_name_ != name)
    {
        track->setName(name);
        track->rebuild(true);
        emit CTrackDB::self().sigModified();
        emit CTrackDB::self().sigModified(track->getKey());

        palette.setColor(QPalette::Base, QColor(128, 255, 128));
    }


    lineName->setPalette(palette);
}

void CTrackEditWidget::slotFilter()
{
    if(track.isNull()) return;

    CDlgTrackFilter dlg(*track, this);
    dlg.exec();
}


void CTrackEditWidget::slotReset()
{
    if(track.isNull()) return;
    originator = true;

    QList<CTrack::pt_t>& trkpts           = track->getTrackPoints();
    QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();
    while(trkpt != trkpts.end())
    {
        trkpt->flags &= ~CTrack::pt_t::eDeleted;
        trkpt->lon = trkpt->_lon;
        trkpt->lat = trkpt->_lat;
        trkpt->ele = trkpt->_ele;

        ++trkpt;
    }

    originator = false;

    track->rebuild(true);
    emit CTrackDB::self().sigModified();
    emit CTrackDB::self().sigModified(track->getKey());
}

void CTrackEditWidget::slotDelete()
{
    if(track.isNull()) return;
    originator = true;

    if(QMessageBox::warning(0,tr("Remove track points ...")
        ,tr("You are about to remove hidden track points permanently. If you press 'yes', all information will be lost.")
        ,QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
    {
        return;
    }

    QList<CTrack::pt_t>& trkpts           = track->getTrackPoints();
    QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();
    while(trkpt != trkpts.end())
    {

        if(trkpt->flags & CTrack::pt_t::eDeleted)
        {
            if ( trkpt->editItem )
            {
                int idx = treePoints->indexOfTopLevelItem((CTrackTreeWidgetItem *)trkpt->editItem.data());
                if ( idx != -1 )
                {
                    treePoints->takeTopLevelItem(idx);
                }
                delete (CTrackTreeWidgetItem *)trkpt->editItem.data();
            }
            trkpt = trkpts.erase(trkpt);
        }
        else
        {
            ++trkpt;
        }
    }
    originator = false;

    track->rebuild(true);
    emit CTrackDB::self().sigModified();
    emit CTrackDB::self().sigModified(track->getKey());
}

void CTrackEditWidget::slotCurrentChanged(int idx)
{
    if(idx == 1)
    {
        slotWptChanged();
    }
}

static bool qSortWptLessDistance(CTrack::wpt_t& p1, CTrack::wpt_t& p2)
{
    return p1.trkpt.distance < p2.trkpt.distance;
}



void CTrackEditWidget::slotWptChanged()
{
    textStages->clear();

    if(track.isNull()) return;

    // get waypoints near track
    QList<CTrack::wpt_t> wpts;
    track->scaleWpt2Track(wpts);
    QList<CTrack::wpt_t>::iterator wpt = wpts.begin();
    while(wpt != wpts.end())
    {
        if(wpt->d > WPT_TO_TRACK_DIST)
        {
            wpt = wpts.erase(wpt);
            continue;
        }
        ++wpt;
    }


    if(wpts.isEmpty())
    {
        tabWidget->setTabEnabled(1, false);
        return;
    }

    updateStages(wpts);

}

#define CHAR_PER_LINE 120
#define ROOT_FRAME_MARGIN 5


void CTrackEditWidget::updateStages(QList<CTrack::wpt_t>& wpts)
{
    tabWidget->setTabEnabled(1, true);
    qSort(wpts.begin(), wpts.end(), qSortWptLessDistance);

    // resize font
    textStages->document()->setTextWidth(textStages->size().width() - 20);
    QTextDocument * doc = textStages->document();
    QFontMetrics fm(QFont(textStages->font().family(),10));

    int w = doc->textWidth();
    int pointSize = ((10 * (w - 2 * ROOT_FRAME_MARGIN)) / (CHAR_PER_LINE *  fm.width("X")));
    if(pointSize == 0) return;
    QFont f = textStages->font();
    f.setPointSize(pointSize);
    textStages->setFont(f);


    // copied from CDiaryEdit
    QTextCharFormat fmtCharStandard;
    fmtCharStandard.setFont(f);

    QTextCharFormat fmtCharShade;
    fmtCharShade.setFont(f);
    fmtCharShade.setBackground(QColor(150,150,255));

    QTextCharFormat fmtCharHeader;
    fmtCharHeader.setFont(f);
    fmtCharHeader.setBackground(Qt::darkBlue);
    fmtCharHeader.setFontWeight(QFont::Bold);
    fmtCharHeader.setForeground(Qt::white);

    QTextBlockFormat fmtBlockStandard;
    fmtBlockStandard.setTopMargin(10);
    fmtBlockStandard.setBottomMargin(10);
    fmtBlockStandard.setAlignment(Qt::AlignJustify);

    QTextFrameFormat fmtFrameStandard;
    fmtFrameStandard.setTopMargin(5);
    fmtFrameStandard.setBottomMargin(5);
    fmtFrameStandard.setWidth(w - 2 * ROOT_FRAME_MARGIN);

    QTextFrameFormat fmtFrameRoot;
    fmtFrameRoot.setTopMargin(ROOT_FRAME_MARGIN);
    fmtFrameRoot.setBottomMargin(ROOT_FRAME_MARGIN);
    fmtFrameRoot.setLeftMargin(ROOT_FRAME_MARGIN);
    fmtFrameRoot.setRightMargin(ROOT_FRAME_MARGIN);

    QTextTableFormat fmtTableStandard;
    fmtTableStandard.setBorder(0);
    fmtTableStandard.setBorderBrush(Qt::black);
    fmtTableStandard.setCellPadding(4);
    fmtTableStandard.setCellSpacing(0);
    fmtTableStandard.setHeaderRowCount(1);
    fmtTableStandard.setTopMargin(10);
    fmtTableStandard.setBottomMargin(20);
    fmtTableStandard.setWidth(w - 2 * ROOT_FRAME_MARGIN);

    QVector<QTextLength> constraints;
    constraints << QTextLength(QTextLength::FixedLength, 32);
    constraints << QTextLength(QTextLength::VariableLength, 50);
    constraints << QTextLength(QTextLength::VariableLength, 30);
    constraints << QTextLength(QTextLength::VariableLength, 30);
    constraints << QTextLength(QTextLength::VariableLength, 100);
    fmtTableStandard.setColumnWidthConstraints(constraints);

    doc->rootFrame()->setFrameFormat(fmtFrameRoot);
    QTextCursor cursor = doc->rootFrame()->firstCursorPosition();

    QTextTable * table = cursor.insertTable(wpts.count()+1+2, eMax, fmtTableStandard);
    table->cellAt(0,eSym).setFormat(fmtCharHeader);
    table->cellAt(0,eEle).setFormat(fmtCharHeader);
    table->cellAt(0,eToLast).setFormat(fmtCharHeader);
    table->cellAt(0,eTotal).setFormat(fmtCharHeader);
    table->cellAt(0,eInfo).setFormat(fmtCharHeader);
    table->cellAt(0,eComment).setFormat(fmtCharHeader);

    table->cellAt(0,eInfo).firstCursorPosition().insertText(tr("Info"));
    table->cellAt(0,eEle).firstCursorPosition().insertText(tr("Ele. wpt/trk"));
    table->cellAt(0,eToLast).firstCursorPosition().insertText(tr("to Last"));
    table->cellAt(0,eTotal).firstCursorPosition().insertText(tr("Total"));
    table->cellAt(0,eComment).firstCursorPosition().insertText(tr("Comment"));

    QString val, val2, unit;
    table->cellAt(1,eSym).firstCursorPosition().insertImage(":/icons/face-plain.png");
    table->cellAt(1,eInfo).firstCursorPosition().insertText(tr("Start"), fmtCharStandard);
    IUnit::self().meter2distance(0,val,unit);
    table->cellAt(1,eToLast).firstCursorPosition().insertText(tr("%1 %2").arg(val).arg(unit), fmtCharStandard);    
    table->cellAt(1,eTotal).firstCursorPosition().insertText(tr("%1 %2").arg(val).arg(unit), fmtCharStandard);
    if(track->getStartElevation() != WPT_NOFLOAT)
    {
        IUnit::self().meter2elevation(track->getStartElevation(),val,unit);
    }
    else
    {
        val     = "-";
        unit    = "";
    }
    table->cellAt(1,eEle).firstCursorPosition().insertText(tr("-/%1 %2").arg(val).arg(unit), fmtCharStandard);
    table->cellAt(1,eComment).firstCursorPosition().insertText(tr("Start of track."), fmtCharStandard);

    int cnt = 2;

    float   distLast = 0;
    int     timeLast = track->getStartTimestamp().toTime_t();
    float   ascLast  = 0;
    float   dscLast  = 0;

    foreach(const CTrack::wpt_t& wpt, wpts)
    {
        if(!(cnt & 0x1))
        {
            table->cellAt(cnt,eSym).setFormat(fmtCharShade);
            table->cellAt(cnt,eEle).setFormat(fmtCharShade);
            table->cellAt(cnt,eToLast).setFormat(fmtCharShade);
            table->cellAt(cnt,eTotal).setFormat(fmtCharShade);
            table->cellAt(cnt,eInfo).setFormat(fmtCharShade);
            table->cellAt(cnt,eComment).setFormat(fmtCharShade);
        }

        table->cellAt(cnt,eSym).firstCursorPosition().insertImage(wpt.wpt->getIcon().toImage().scaledToWidth(16, Qt::SmoothTransformation));
        table->cellAt(cnt,eInfo).firstCursorPosition().insertText(wpt.wpt->getName(), fmtCharStandard);

        QString strTimeToLast;
        QString strTimeTotal;
        QString strDistToLast;
        QString strDistTotal;
        QString strAscToLast;
        QString strAscTotal;

        quint32 timestamp = wpt.trkpt.timestamp;
        if(timeLast && timestamp)
        {
            quint32 t1s     = timestamp - timeLast;
            quint32 t1h     = qreal(t1s)/3600;
            quint32 t1m     = quint32(qreal(t1s - t1h * 3600)/60  + 0.5);
            strTimeToLast   = tr("%3 %1:%2 h").arg(t1h).arg(t1m, 2, 10, QChar('0')).arg(QChar(0x231a));

            quint32 t2s     = timestamp - track->getStartTimestamp().toTime_t();
            quint32 t2h     = qreal(t2s)/3600;
            quint32 t2m     = quint32(qreal(t2s - t2h * 3600)/60  + 0.5);
            strTimeTotal    = tr("%3 %1:%2 h").arg(t2h).arg(t2m, 2, 10, QChar('0')).arg(QChar(0x231a));

            timeLast = timestamp;
        }

        IUnit::self().meter2distance(wpt.trkpt.distance - distLast, val, unit);
        strDistToLast = tr("%1 %2").arg(val).arg(unit);

        IUnit::self().meter2distance(wpt.trkpt.distance, val, unit);
        strDistTotal = tr("%1 %2").arg(val).arg(unit);

        IUnit::self().meter2elevation(wpt.trkpt.ascend - ascLast, val, unit);
        strAscToLast  = tr("%1%2 %3 ").arg(QChar(0x2197)).arg(val).arg(unit);
        IUnit::self().meter2elevation(wpt.trkpt.descend - dscLast, val, unit);
        strAscToLast += tr("%1%2 %3").arg(QChar(0x2198)).arg(val).arg(unit);

        IUnit::self().meter2elevation(wpt.trkpt.ascend, val, unit);
        strAscTotal  = tr("%1%2 %3 ").arg(QChar(0x2197)).arg(val).arg(unit);
        IUnit::self().meter2elevation(wpt.trkpt.descend, val, unit);
        strAscTotal += tr("%1%2 %3").arg(QChar(0x2198)).arg(val).arg(unit);

        if(wpt.wpt->ele != WPT_NOFLOAT)
        {
            IUnit::self().meter2elevation(wpt.wpt->ele, val, unit);
        }
        else
        {
            val = "-";
        }
        if(wpt.trkpt.ele != WPT_NOFLOAT)
        {
            IUnit::self().meter2elevation(wpt.trkpt.ele, val2, unit);
        }
        else
        {
            val2 = "-";
        }

        table->cellAt(cnt,eEle).firstCursorPosition().insertText(tr("%1/%2 %3").arg(val).arg(val2).arg(unit), fmtCharStandard);
        table->cellAt(cnt,eToLast).firstCursorPosition().insertText(tr("%1 %2\n%3").arg(strDistToLast).arg(strTimeToLast).arg(strAscToLast), fmtCharStandard);
        table->cellAt(cnt,eTotal).firstCursorPosition().insertText(tr("%1 %2\n%3").arg(strDistTotal).arg(strTimeTotal).arg(strAscTotal), fmtCharStandard);

        QString comment = wpt.wpt->getComment();
        comment.remove(QRegExp("<head.*[^>]*><\\/head>"));
        comment.remove(QRegExp("<[^>]*>"));
        comment = comment.simplified();
        table->cellAt(cnt,eComment).firstCursorPosition().insertText(comment, fmtCharStandard);

        distLast    = wpt.trkpt.distance;
        ascLast     = wpt.trkpt.ascend;
        dscLast     = wpt.trkpt.descend;

        cnt++;
    }


    if(!(cnt & 0x1))
    {
        table->cellAt(cnt,eSym).setFormat(fmtCharShade);
        table->cellAt(cnt,eEle).setFormat(fmtCharShade);
        table->cellAt(cnt,eToLast).setFormat(fmtCharShade);
        table->cellAt(cnt,eTotal).setFormat(fmtCharShade);
        table->cellAt(cnt,eInfo).setFormat(fmtCharShade);
        table->cellAt(cnt,eComment).setFormat(fmtCharShade);
    }

    table->cellAt(cnt,eSym).firstCursorPosition().insertImage(":/icons/face-laugh.png");
    table->cellAt(cnt,eInfo).firstCursorPosition().insertText(tr("End"), fmtCharStandard);
    table->cellAt(cnt,eComment).firstCursorPosition().insertText(tr("End of track."), fmtCharStandard);

    if(track->getEndElevation() != WPT_NOFLOAT)
    {
        IUnit::self().meter2elevation(track->getEndElevation(),val,unit);
    }
    else
    {
        val  = "-";
        unit = "";
    }
    table->cellAt(cnt,eEle).firstCursorPosition().insertText(tr("-/%1 %2").arg(val).arg(unit), fmtCharStandard);

    QString strTimeToLast;
    QString strTimeTotal;
    QString strDistToLast;
    QString strDistTotal;
    QString strAscToLast;
    QString strAscTotal;

    quint32 timestamp = track->getEndTimestamp().toTime_t();
    if(timeLast && timestamp)
    {
        quint32 t1s     = timestamp - timeLast;
        quint32 t1h     = qreal(t1s)/3600;
        quint32 t1m     = quint32(qreal(t1s - t1h * 3600)/60  + 0.5);
        strTimeToLast   = tr("%3 %1:%2 h").arg(t1h).arg(t1m, 2, 10, QChar('0')).arg(QChar(0x231a));

        quint32 t2s     = timestamp - track->getStartTimestamp().toTime_t();
        quint32 t2h     = qreal(t2s)/3600;
        quint32 t2m     = quint32(qreal(t2s - t2h * 3600)/60  + 0.5);
        strTimeTotal   = tr("%3 %1:%2 h").arg(t2h).arg(t2m, 2, 10, QChar('0')).arg(QChar(0x231a));

        timeLast = timestamp;
    }

    IUnit::self().meter2distance(track->getTotalDistance() - distLast, val, unit);
    strDistToLast = tr("%1 %2").arg(val).arg(unit);

    IUnit::self().meter2distance(track->getTotalDistance(), val, unit);
    strDistTotal = tr("%1 %2").arg(val).arg(unit);

    IUnit::self().meter2elevation(track->getAscend() - ascLast, val, unit);
    strAscToLast  = tr("%1%2 %3 ").arg(QChar(0x2197)).arg(val).arg(unit);
    IUnit::self().meter2elevation(track->getDescend() - dscLast, val, unit);
    strAscToLast += tr("%1%2 %3").arg(QChar(0x2198)).arg(val).arg(unit);

    IUnit::self().meter2elevation(track->getAscend(), val, unit);
    strAscTotal  = tr("%1%2 %3 ").arg(QChar(0x2197)).arg(val).arg(unit);
    IUnit::self().meter2elevation(track->getDescend(), val, unit);
    strAscTotal += tr("%1%2 %3").arg(QChar(0x2198)).arg(val).arg(unit);

    table->cellAt(cnt,eToLast).firstCursorPosition().insertText(tr("%1 %2\n%3").arg(strDistToLast).arg(strTimeToLast).arg(strAscToLast), fmtCharStandard);
    table->cellAt(cnt,eTotal).firstCursorPosition().insertText(tr("%1 %2\n%3").arg(strDistTotal).arg(strTimeTotal).arg(strAscTotal), fmtCharStandard);

}
