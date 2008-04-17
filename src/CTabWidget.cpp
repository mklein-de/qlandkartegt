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

#include "CTabWidget.h"

#include <QtGui>

CTabWidget::CTabWidget(QWidget * parent)
    : QTabWidget(parent)
{
     tabBar()->hide();
}

CTabWidget::~CTabWidget()
{

}

void CTabWidget::addTab(QWidget * w, const QString& label)
{
    QTabWidget::addTab(w,label);
    setCurrentWidget(w);

    count() > 1 ? tabBar()->show() : tabBar()->hide();
}

void CTabWidget::delTab(QWidget * w)
{
    delete w;
    count() > 1 ? tabBar()->show() : tabBar()->hide();
}
