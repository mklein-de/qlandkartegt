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

#include "CLiveLogToolWidget.h"
#include "CLiveLogDB.h"

#include <QtGui>

CLiveLogToolWidget::CLiveLogToolWidget(QTabWidget * parent)
:QWidget(parent)
{
    setupUi(this);

    setObjectName("LiveLog");
    parent->addTab(this,QIcon(":/icons/iconLiveLog16x16"),"");
    parent->setTabToolTip(parent->indexOf(this), tr("Live Log"));

    connect(&CLiveLogDB::self(), SIGNAL(sigChanged()), this, SLOT(slotChanged()));

    labelCenter->setPixmap(QPixmap(":/icons/iconLock16x16"));
    labelCenter->hide();
}


CLiveLogToolWidget::~CLiveLogToolWidget()
{

}


void CLiveLogToolWidget::slotChanged()
{
}
