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

#include "CTrackEditWidget.h"
#include "CTrack.h"

#include <QtGui>

CTrackEditWidget::CTrackEditWidget(QWidget * parent)
    : QWidget(parent)
{
    setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose,true);

    toolExit->setIcon(QIcon(":/icons/iconExit16x16.png"));
    connect(toolExit, SIGNAL(clicked()), this, SLOT(close()));

    QPixmap icon(16,8);
    for(int i=0; i < 17; ++i) {
        icon.fill(CTrack::colors[i]);
        comboColor->addItem(icon,"",QVariant(i));
    }
}

CTrackEditWidget::~CTrackEditWidget()
{

}

void CTrackEditWidget::slotSetTrack(CTrack * t)
{
    track = t;
    lineName->setText(track->getName());
    comboColor->setCurrentIndex(track->getColorIdx());
}
