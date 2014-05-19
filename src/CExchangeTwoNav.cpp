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
#include "CExchangeTwoNav.h"

#include <QtDBus>

CTwoNavTreeWidgetItem::CTwoNavTreeWidgetItem(const QString& id, QTreeWidget *parent)
    : IDeviceTreeWidgetItem(id,parent)
{
    setIcon(0, QIcon("://icons/iconDeviceTwoNav16x16.png"));
}


CExchangeTwoNav::CExchangeTwoNav(QTreeWidget * treeWidget, QObject * parent)
    : IExchange(treeWidget,parent)
{

    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2",
                   "/org/freedesktop/UDisks2",
                   "org.freedesktop.DBus.ObjectManager",
                   "InterfacesAdded",
                   this,
                   SLOT(slotDeviceAdded(QDBusObjectPath,QVariantMap)));

    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2",
                   "/org/freedesktop/UDisks2",
                   "org.freedesktop.DBus.ObjectManager",
                   "InterfacesRemoved",
                   this,
                   SLOT(slotDeviceRemoved(QDBusObjectPath,QStringList)));


    QTimer::singleShot(1000, this, SLOT(slotUpdate()));

}

CExchangeTwoNav::~CExchangeTwoNav()
{

}

void CExchangeTwoNav::slotUpdate()
{

    QList<QDBusObjectPath> paths;
    QDBusMessage call = QDBusMessage::createMethodCall("org.freedesktop.UDisks2",
                                                       "/org/freedesktop/UDisks2/block_devices",
                                                       "org.freedesktop.DBus.Introspectable",
                                                       "Introspect");
    QDBusPendingReply<QString> reply = QDBusConnection::systemBus().call(call);

    if (!reply.isValid())
    {
        qWarning("UDisks2Manager: error: %s", qPrintable(reply.error().name()));
        return;
    }

    QXmlStreamReader xml(reply.value());
    while (!xml.atEnd())
    {
        xml.readNext();
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name().toString() == "node" )
        {
            QString name = xml.attributes().value("name").toString();
            if(!name.isEmpty())
                paths << QDBusObjectPath("/org/freedesktop/UDisks2/block_devices/" + name);
        }
    }

    foreach (QDBusObjectPath i, paths)
    {
        slotDeviceAdded(i, QVariantMap());
    }

//    qDebug() << mDevicesByPath;
}

void CExchangeTwoNav::slotDeviceAdded(const QDBusObjectPath& path, const QVariantMap& map)
{
    qDebug() << "-----------slotDeviceAdded----------";
    qDebug() << path.path() << map;

    QString device = checkForDevice(path, "GENERAL");
    if(!device.isEmpty())
    {
        CTwoNavTreeWidgetItem * item = new CTwoNavTreeWidgetItem(path.path(), treeWidget);
        item->setText(0, "TwoNav " + device);
    }
}

void CExchangeTwoNav::slotDeviceRemoved(const QDBusObjectPath& path, const QStringList& list)
{
    qDebug() << "-----------dbusDeviceRemoved----------";
    qDebug() << path.path() << list;
    if(!path.path().startsWith("/org/freedesktop/UDisks2/block_devices/"))
    {
        return;
    }

    const int N = treeWidget->topLevelItemCount();
    for(int i = 0; i<N; i++)
    {
        CTwoNavTreeWidgetItem * item = dynamic_cast<CTwoNavTreeWidgetItem*>(treeWidget->topLevelItem(i));
        if(item && item->getId() == path.path())
        {
            delete item;
            return;
        }
    }
}

