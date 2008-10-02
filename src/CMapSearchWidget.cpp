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

#include "CMapSearchWidget.h"
#include "CMainWindow.h"
#include "CCanvas.h"
#include "GeoMath.h"

#include <QtGui>

CMapSearchWidget::CMapSearchWidget(QWidget * parent)
: QWidget(parent)
{
    setupUi(this);
    setObjectName("CMapSearchWidget");
    setAttribute(Qt::WA_DeleteOnClose,true);
    toolExit->setIcon(QIcon(":/icons/iconExit16x16.png"));
    connect(toolExit, SIGNAL(clicked()), this, SLOT(close()));

    connect(pushArea, SIGNAL(clicked()), this, SLOT(slotSelectArea()));
    connect(pushMask, SIGNAL(clicked()), this, SLOT(slotSelectMask()));
    connect(pushSearch, SIGNAL(clicked()), this, SLOT(slotSearch()));

}

CMapSearchWidget::~CMapSearchWidget()
{

}

void CMapSearchWidget::slotSelectArea()
{
   theMainWindow->getCanvas()->setMouseMode(CCanvas::eMouseSelectArea);
}

void CMapSearchWidget::slotSelectMask()
{

}

void CMapSearchWidget::slotSearch()
{

}

void CMapSearchWidget::setArea(const CMapSelection& ms)
{
    area = ms;
    QString pos1, pos2;

    GPS_Math_Deg_To_Str(ms.lon1 * RAD_TO_DEG, ms.lat1 * RAD_TO_DEG, pos1);
    GPS_Math_Deg_To_Str(ms.lon2 * RAD_TO_DEG, ms.lat2 * RAD_TO_DEG, pos2);

    labelArea->setText(QString("%1\n%2\n%3").arg(ms.description).arg(pos1).arg(pos2));
}
