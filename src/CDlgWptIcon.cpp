/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de

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

#include "CDlgWptIcon.h"
#include "WptIcons.h"

#include <QtGui>

CDlgWptIcon::CDlgWptIcon(QToolButton& but)
: QDialog(&but)
, button(but)
{
    setupUi(this);

    QString currentIcon = button.objectName();
    QListWidgetItem * currentItem = 0;


    const wpt_icon_t * icon = getWptIcons();
    while(icon->name != 0) {
        QListWidgetItem * item = new QListWidgetItem(loadIcon(icon->icon), icon->name, listIcons);
        if(currentIcon == icon->name) {
            currentItem = item;
        }
        ++icon;
    }

    if(currentItem) {
        listIcons->setCurrentItem(currentItem);
        listIcons->scrollToItem(currentItem);
    }

    connect(listIcons, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slotItemClicked(QListWidgetItem*)));
}


CDlgWptIcon::~CDlgWptIcon()
{

}


void CDlgWptIcon::slotItemClicked(QListWidgetItem * item)
{

    button.setIcon(item->icon());
    button.setObjectName(item->text());
    button.setToolTip(item->text());
    accept();
}
