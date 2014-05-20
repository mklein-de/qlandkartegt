/**********************************************************************************************
    Copyright (C) 2014 Oliver Eichler oliver.eichler@gmx.de

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

**********************************************************************************************/
#include "CExchangeGarmin.h"

#include <QtDBus>

CGarminTreeWidgetItem::CGarminTreeWidgetItem(const QString& id, QTreeWidget *parent)
    : IDeviceTreeWidgetItem(id,parent)
{
    setIcon(0, QIcon("://icons/iconDeviceGarmin16x16.png"));
}

CExchangeGarmin::CExchangeGarmin(QTreeWidget * treeWidget, QObject * parent)
    : IExchange("Garmin", treeWidget,parent)
{

}

CExchangeGarmin::~CExchangeGarmin()
{

}

void CExchangeGarmin::slotDeviceAdded(const QDBusObjectPath& path, const QVariantMap& map)
{
    qDebug() << "-----------CExchangeGarmin::slotDeviceAdded----------";
    qDebug() << path.path() << map;

    QString device = checkForDevice(path);
    if(!device.isEmpty())
    {
        CGarminTreeWidgetItem * item = new CGarminTreeWidgetItem(path.path(), treeWidget);
        item->setText(0, device);

    }
}

