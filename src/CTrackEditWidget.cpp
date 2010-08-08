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
, num_of_ext(0)
, Vspace(0)
, tabstat(0)
, no_ext_info_stat(0)
, count(0)
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

    QPixmap icon(16,8);
    for(int i=0; i < 17; ++i)
    {
        icon.fill(CTrack::lineColors[i]);
        comboColor->addItem(icon,"",QVariant(i));
    }

    connect(checkRemoveDelTrkPt,SIGNAL(clicked(bool)),this,SLOT(slotCheckRemove(bool)));
    connect(checkResetDelTrkPt,SIGNAL(clicked(bool)),this,SLOT(slotCheckReset(bool)));
    connect(buttonBox,SIGNAL(clicked (QAbstractButton*)),this,SLOT(slotApply()));
    connect(treePoints,SIGNAL(itemSelectionChanged()),this,SLOT(slotPointSelectionChanged()));
    connect(treePoints,SIGNAL(itemClicked(QTreeWidgetItem*,int)),this,SLOT(slotPointSelection(QTreeWidgetItem*)));

#ifdef GPX_EXTENSIONS
    //------------------------------------
    //TODO: Extra Icon fr Extension & Connect dazu

    toolGraphExtensions->setIcon(QIcon(":/icons/iconExtensions16x16.png"));
    connect(toolGraphExtensions, SIGNAL(clicked()), this, SLOT(slotToggleExtensionsGraph()));

    //TODO: Icon for Google maps

    toolGoogleMaps->setIcon(QIcon(":/icons/iconGoogleMaps16x16.png"));
    connect(toolGoogleMaps, SIGNAL(clicked()), this, SLOT(slotGoogleMaps()));

    //--------------------------------------

    //TODO: checkboxes to switch on/off standard columns
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
    if(!trackStatTrainee.isNull())
    {
        delete trackStatTrainee;
    }
#ifdef GPX_EXTENSIONS
    num_of_ext  = track->tr_ext.set.toList().size();
    for(int i=0; i < num_of_ext; ++i)
    {
        //TODO: delete all extension tabs and reset tab counter and tabs list
        if (tabs.size() != 0)
        {
            if (tabs[i])
            {
                delete tabs[i];
            }
            if (i == num_of_ext-1)
            {
                tabstat = 0;
                tabs.clear();
            }
        }

    }
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

    if(track)
    {
        disconnect(track,SIGNAL(sigChanged()), this, SLOT(slotUpdate()));
        disconnect(track,SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));

        // clean view
        QList<CTrack::pt_t>& trkpts           = track->getTrackPoints();
        QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();
        while(trkpt != trkpts.end())
        {
            trkpt->editItem = 0;
            ++trkpt;
        }
        treePoints->clear();     // this also delete the items

#ifdef GPX_EXTENSIONS
        //------------------------------------------------------------------------------------
        //TODO: delete checkboxes & spacer
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

    //TODO: create for every extension a checkbox and column and link them together
#ifdef GPX_EXTENSIONS
                                 //Anzahl der Extensions
    num_of_ext      = track->tr_ext.set.toList().size();
                                 //Namen der Extensions
    names_of_ext    = track->tr_ext.set.toList();

    if (num_of_ext)
    {

        gridLayout_Extensions->removeWidget(label);

        //checkboxes setzten
        for(int i=0; i < num_of_ext; ++i)
        {

                                 //Namen der Extentions pro i in Variable
            QString name        = names_of_ext[i];
                                 //Name des objects
            QString obj_name    = QString("%1").arg(eMaxColumn+i);

                                 // Check box generieren, checken und mit Name versehen
            QCheckBox *CheckBoxMake;
            CheckBoxMake = new QCheckBox(name);
            CheckBoxMake->setChecked(true);
            CheckBoxMake->setObjectName(obj_name);
            //CheckBoxMake->setToolTip(obj_name);
            c_boxes.insert(i, CheckBoxMake);
            verticalLayout_Extensions->addWidget(CheckBoxMake, i, 0);


                                 // hier wird hochgezhlt
            int number_of_column = eMaxColumn + i;

                                 //Spalten in die Trackliste einfgen
            QTreeWidgetItem *___qtreewidgetitem = treePoints->headerItem();
            ___qtreewidgetitem->setText(number_of_column, name);

                                 // Connect zwischen checkbox und column
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
    num_of_ext = track->tr_ext.set.toList().size();
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

#ifdef GPX_EXTENSIONS
                                 //Anzahl der Extensions
    num_of_ext =  track->tr_ext.set.toList().size();
                                 //Namen der Extensions
    names_of_ext = track->tr_ext.set.toList();
#endif

    //TODO: Declare Google maps parameter
    QString gmaps;

    while(trkpt != trkpts.end())
    {
        CTrackTreeWidgetItem * item;
        if ( !trkpt->editItem )
        {
            trkpt->editItem = (QObject*)new CTrackTreeWidgetItem(treePoints);
            item = (CTrackTreeWidgetItem *)trkpt->editItem;
            item->setTextAlignment(eNum,Qt::AlignLeft);
            item->setTextAlignment(eAltitude,Qt::AlignRight);
            item->setTextAlignment(eDelta,Qt::AlignRight);
            item->setTextAlignment(eAzimuth,Qt::AlignRight);
            item->setTextAlignment(eDistance,Qt::AlignRight);
            item->setTextAlignment(eAscend,Qt::AlignRight);
            item->setTextAlignment(eDescend,Qt::AlignRight);
            item->setTextAlignment(eSpeed,Qt::AlignRight);

            trkpt->flags.setChanged(true);
        }

        if ( !trkpt->flags.isChanged() )
        {
            ++trkpt;
            continue;
        }

        item = (CTrackTreeWidgetItem *)trkpt->editItem;
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

        //QDesktopServices::openUrl(QUrl(gmaps));	//Opens URL directly in Browser

        // position

        GPS_Math_Deg_To_Str(trkpt->lon, trkpt->lat, str);
        item->setText(ePosition,str);

        /*
                if (count == 100) {
                //TODO: Google maps link

                gmaps = tr("http://maps.google.com/maps?f=l&t=h&z=16&om=1&g=&q=%1,%2(%3. Trackpoint)").arg(trkpt->lat).arg(trkpt->lon).arg(trkpt->idx);

                QLabel *gmlabel = new QLabel(this);
                gmlabel->setText("<a href=\""+gmaps+"\">"+str+"</a>");

                gmlabel->setToolTip("click to show ["+str+"] on google maps");
                gmlabel->setOpenExternalLinks(true);
                gmlabel->setAutoFillBackground(true);

                treePoints->setItemWidget(item, ePosition, gmlabel);

                count = 0;

                }
                else {
                    item->setText(ePosition,str);
                    count++;
                }

        */

        //--------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------------------------------------------
        // TODO: Ext. einfgen

#ifdef GPX_EXTENSIONS
        //Trackliste Zellen fllen
        for(int i=0; i < num_of_ext; ++i)
        {
            QString end;
            int col = eMaxColumn+i;

                                 //Name der Ext.
            QString nam = names_of_ext[i];

                                 //Wert der Ext.
            QString val = trkpt->gpx_exts.getValue(nam);

            if (val != "")  {end = val;}
            else            {end = "-";}

                                 //Einfgen
            item->setText(col,end);
                                 // formatieren
            item->setTextAlignment(col,Qt::AlignRight);

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


void CTrackEditWidget::slotCheckReset(bool checked)
{
    if(checked)
    {
        checkRemoveDelTrkPt->setChecked(false);
    }
}


void CTrackEditWidget::slotCheckRemove(bool checked)
{
    if(checked)
    {
        checkResetDelTrkPt->setChecked(false);
    }
}


void CTrackEditWidget::slotApply()
{
    if(track.isNull()) return;

    originator = true;

    if(checkResetDelTrkPt->isChecked())
    {
        QList<CTrack::pt_t>& trkpts           = track->getTrackPoints();
        QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();
        while(trkpt != trkpts.end())
        {
            trkpt->flags &= ~CTrack::pt_t::eDeleted;
            ++trkpt;
        }
        checkResetDelTrkPt->setChecked(false);
        originator = false;
    }

    if(checkRemoveDelTrkPt->isChecked())
    {
        QMessageBox::warning(0,tr("Remove track points ...")
            ,tr("You are about to remove purged track points permanently. If you press 'yes', all information will be lost.")
            ,QMessageBox::Yes|QMessageBox::No);
        QList<CTrack::pt_t>& trkpts           = track->getTrackPoints();
        QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();
        while(trkpt != trkpts.end())
        {

            if(trkpt->flags & CTrack::pt_t::eDeleted)
            {
                if ( trkpt->editItem )
                {
                    int idx = treePoints->indexOfTopLevelItem((CTrackTreeWidgetItem *)trkpt->editItem);
                    if ( idx != -1 )
                        treePoints->takeTopLevelItem(idx);
                    delete (CTrackTreeWidgetItem *)trkpt->editItem;
                    trkpt->editItem = 0;
                }
                trkpt = trkpts.erase(trkpt);
            }
            else
            {
                ++trkpt;
            }
        }
        checkRemoveDelTrkPt->setChecked(false);
        originator = false;
    }

    track->setName(lineName->text());
    track->setColor(comboColor->currentIndex());
    track->rebuild(true);
    originator = false;

    emit CTrackDB::self().sigModified();
    emit CTrackDB::self().sigModified(track->key());
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
    track->setPointOfFocus(item->data(0,Qt::UserRole).toInt());
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
    emit CTrackDB::self().sigModified(track->key());
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
                                 //TODO: TIME BUTTON
        trackStatSpeedTime = new CTrackStatSpeedWidget(ITrackStat::eOverTime, this);
        theMainWindow->getCanvasTab()->setTabPosition(QTabWidget::South);
                                 //TAB hinzufgen
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
//TODO: method to show & hide the extensions graphs
void CTrackEditWidget::slotToggleExtensionsGraph()
{

                                 //Anzahl der Extensions
    num_of_ext      = track->tr_ext.set.toList().size();
                                 //Namen der Extensions
    names_of_ext    = track->tr_ext.set.toList();

    for(int i=0; i < num_of_ext; ++i)
    {
        QString name = names_of_ext[i];

        if (tabstat == 0)
        {
                                 //Ext tab ber die zeit
            tab = new CTrackStatExtensionWidget(ITrackStat::eOverTime, this);
            tab->setObjectName(name);
            tab->setToolTip(name);

                                 //add Tab
            theMainWindow->getCanvasTab()->addTab(tab, name+"/t");
            tabs.insert(i, tab); //add Tab index to list for further handling

            if (i == num_of_ext-1) {tabstat = 1;}
        }
        else if (tabstat == 1)

        {
            //delete all extension tabs and reset tab counter and tabs list
            if(tabs[i] != 0)
            {
                delete tabs[i];
                //theMainWindow->getCanvasTab()->removeTab(i);
            }
            if (i == num_of_ext-1)
            {
                tabstat = 0;
                tabs.clear();
            }
        }

    }

    //Tab Settings
    theMainWindow->getCanvasTab()->setTabPosition(QTabWidget::South);
    theMainWindow->getCanvasTab()->setMovable(true);

    //TODO: make tabs closeable

    theMainWindow->getCanvasTab()->setTabsClosable(true);

    if (tabstat == 1)
    {
        connect(theMainWindow->getCanvasTab(),SIGNAL(tabCloseRequested(int)),this,SLOT(slotKillTab(int)));
    }
    else
    {
        disconnect(theMainWindow->getCanvasTab(),SIGNAL(tabCloseRequested(int)), this, SLOT(slotKillTab(int)));
    }

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
        if(checked == true) {CTrackEditWidget::treePoints->showColumn(0);}
        else                {CTrackEditWidget::treePoints->hideColumn(0);}
    }
                                 //1. checkBox_tim - time
    else if (name_std_is == "checkBox_tim")
    {
        if(checked == true) {CTrackEditWidget::treePoints->showColumn(1);}
        else                {CTrackEditWidget::treePoints->hideColumn(1);}
    }
                                 //2 .checkBox_hig - hight
    else if (name_std_is == "checkBox_hig")
    {
        if(checked == true) {CTrackEditWidget::treePoints->showColumn(2);}
        else                {CTrackEditWidget::treePoints->hideColumn(2);}
    }
                                 //3 .checkBox_dis - distance
    else if (name_std_is == "checkBox_dis")
    {
        if(checked == true) {CTrackEditWidget::treePoints->showColumn(3);}
        else                {CTrackEditWidget::treePoints->hideColumn(3);}
    }
                                 //4 .checkBox_azi - azimuth
    else if (name_std_is == "checkBox_azi")
    {
        if(checked == true) {CTrackEditWidget::treePoints->showColumn(4);}
        else                {CTrackEditWidget::treePoints->hideColumn(4);}
    }
                                 //5 .checkBox_ent - entfernung
    else if (name_std_is == "checkBox_ent")
    {
        if(checked == true) {CTrackEditWidget::treePoints->showColumn(5);}
        else                {CTrackEditWidget::treePoints->hideColumn(5);}
    }
                                 //6 .checkBox_vel - velocity
    else if (name_std_is == "checkBox_vel")
    {
        if(checked == true) {CTrackEditWidget::treePoints->showColumn(6);}
        else                {CTrackEditWidget::treePoints->hideColumn(6);}
    }
                                 //4 .checkBox_suu - summ up
    else if (name_std_is == "checkBox_suu")
    {
        if(checked == true) {CTrackEditWidget::treePoints->showColumn(7);}
        else                {CTrackEditWidget::treePoints->hideColumn(7);}
    }
                                 //8 .checkBox_sud - summ down
    else if (name_std_is == "checkBox_sud")
    {
        if(checked == true) {CTrackEditWidget::treePoints->showColumn(8);}
        else                {CTrackEditWidget::treePoints->hideColumn(8);}
    }
                                 //9 .checkBox_pos - position
    else if (name_std_is == "checkBox_pos")
    {
        if(checked == true) {CTrackEditWidget::treePoints->showColumn(9);}
        else                {CTrackEditWidget::treePoints->hideColumn(9);}
    }
    else
    {                            //sender unknown -> nothing happens
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
