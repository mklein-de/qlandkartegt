/**********************************************************************************************
    Copyright (C) 2010 Oliver Eichler oliver.eichler@gmx.de

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

#include "COverlayDistanceEditWidget.h"
#include "COverlayDistance.h"
#include "IUnit.h"

#include <QtGui>

COverlayDistanceEditWidget::COverlayDistanceEditWidget(QWidget * parent, COverlayDistance * ovl)
: QWidget(parent)
, ovl(ovl)
{
    setupUi(this);

    lineName->setText(ovl->name);
    textComment->setText(ovl->comment);

    labelUnit->setText(IUnit::self().speedunit);
    lineSpeed->setText(QString::number(ovl->speed * IUnit::self().speedfactor));

    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(slotApply()));
    connect(buttonBox->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SLOT(deleteLater()));
}

COverlayDistanceEditWidget::~COverlayDistanceEditWidget()
{

}

void COverlayDistanceEditWidget::slotApply()
{
    ovl->name = lineName->text();
    ovl->comment = textComment->toPlainText();
    ovl->speed = lineSpeed->text().toDouble() / IUnit::self().speedfactor;

    emit ovl->sigChanged();
}
