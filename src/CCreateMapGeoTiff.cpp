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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/

#include "CCreateMapGeoTiff.h"
#include "CMainWindow.h"
#include "CMapDB.h"

#include <QtGui>

CCreateMapGeoTiff * CCreateMapGeoTiff::m_self = 0;

CCreateMapGeoTiff::CCreateMapGeoTiff(QWidget * parent)
: QWidget(parent)
, refcnt(0)
{
    m_self = this;
    setupUi(this);
    labelStep1->setPixmap(QPixmap(":/pics/Step1"));
    labelStep2->setPixmap(QPixmap(":/pics/Step2"));
    labelStep3->setPixmap(QPixmap(":/pics/Step3"));

    connect(pushOpenFile, SIGNAL(clicked()), this, SLOT(slotOpenFile()));
    connect(pushAddRef, SIGNAL(clicked()), this, SLOT(slotAddRef()));
    connect(pushDelRef, SIGNAL(clicked()), this, SLOT(slotDelRef()));
    connect(treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelectionChanged()));

    theMainWindow->getCanvas()->setMouseMode(CCanvas::eMouseMoveRefPoint);
}


CCreateMapGeoTiff::~CCreateMapGeoTiff()
{
    theMainWindow->getCanvas()->setMouseMode(CCanvas::eMouseMoveArea);
    m_self = 0;
}

void CCreateMapGeoTiff::enableStep2()
{
    labelStep2->setEnabled(true);
    treeWidget->setEnabled(true);
    labelDoc2->setEnabled(true);
    pushAddRef->setEnabled(true);
}

void CCreateMapGeoTiff::enableStep3()
{
    labelStep3->setEnabled(true);
    textBrowser->setEnabled(true);
    labelDoc3->setEnabled(true);
}

void CCreateMapGeoTiff::slotOpenFile()
{
    QString filename = QFileDialog::getOpenFileName(0, tr("Open map file..."),"./");
    if(filename.isEmpty()) return;

    CMapDB::self().openMap(filename, *theMainWindow->getCanvas());
    labelInputFile->setText(filename);
    enableStep2();
}

void CCreateMapGeoTiff::slotAddRef()
{
    refpt_t& pt     = refpts[++refcnt];
    pt.item         = new QTreeWidgetItem(treeWidget);

    pt.item->setText(eNum,tr("%1").arg(refcnt));
    pt.item->setData(eNum,Qt::UserRole,refcnt);
    pt.item->setText(eLabel,tr("Ref %1").arg(refcnt));
    pt.item->setText(eLat,tr("???"));
    pt.item->setText(eLon,tr("???"));

    QPoint center   = theMainWindow->getCanvas()->rect().center();
    IMap& map       = CMapDB::self().getMap();
    pt.x            = center.x();
    pt.y            = center.y();
    map.convertPt2M(pt.x,pt.y);

    pt.item->setText(eX,QString::number(pt.x));
    pt.item->setText(eY,QString::number(pt.y));

    pushGoOn->setEnabled(treeWidget->topLevelItemCount() > 2);

    theMainWindow->getCanvas()->update();
}

void CCreateMapGeoTiff::slotDelRef()
{

}

void CCreateMapGeoTiff::slotSelectionChanged()
{

}

void CCreateMapGeoTiff::slotItemDoubleClicked(QTreeWidgetItem * item)
{

}

