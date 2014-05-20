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

#include "IExchange.h"
#include <QtGui>
#include <QtDBus>

IExchange::IExchange(const QString& vendor, QTreeWidget *treeWidget, QObject *parent)
    : QObject(parent)
    , vendor(vendor)
    , treeWidget(treeWidget)
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

IExchange::~IExchange()
{

}

QString IExchange::checkForDevice(const QDBusObjectPath& path)
{
    // ignore all path that are no block devices
    if(!path.path().startsWith("/org/freedesktop/UDisks2/block_devices/"))
    {
        return "";
    }


    // create path of to drive the block device belongs to
    QDBusInterface * blockIface = new QDBusInterface("org.freedesktop.UDisks2",
                                         path.path(),
                                         "org.freedesktop.UDisks2.Block",
                                         QDBusConnection::systemBus(),
                                         this);
    Q_ASSERT(blockIface);
    QDBusObjectPath drive_object = blockIface->property("Drive").value<QDBusObjectPath>();


    // read vendor string attached to drive
    QDBusInterface * driveIface = new QDBusInterface("org.freedesktop.UDisks2",
                                         drive_object.path(),
                                         "org.freedesktop.UDisks2.Drive",
                                         QDBusConnection::systemBus(),
                                         this);
    Q_ASSERT(driveIface);
    QString _vendor_ = driveIface->property("Vendor").toString();

    // vendor must match
    if(_vendor_.toUpper() != vendor.toUpper())
    {
        delete driveIface;
        delete blockIface;
        return "";
    }

    // single out block devices with attached filesystem
    // devices with partitions will have a block device for the drive and one for each partition
    // we are interested in the ones with filesystem as they are the ones to mount
    QDBusMessage call = QDBusMessage::createMethodCall("org.freedesktop.UDisks2",
                                                       path.path(),
                                                       "org.freedesktop.DBus.Introspectable",
                                                       "Introspect");
    QDBusPendingReply<QString> reply = QDBusConnection::systemBus().call(call);

    QXmlStreamReader xml(reply.value());
    while (!xml.atEnd())
    {
        xml.readNext();
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name().toString() == "interface" )
        {

            QString name = xml.attributes().value("name").toString();
            if(name == "org.freedesktop.UDisks2.Filesystem")
            {
                return  driveIface->property("Model").toString();
            }
        }
    }
    delete driveIface;
    delete blockIface;
    return "";
}

void IExchange::slotUpdate()
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
}

void IExchange::slotDeviceRemoved(const QDBusObjectPath& path, const QStringList& list)
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
        IDeviceTreeWidgetItem * item = dynamic_cast<IDeviceTreeWidgetItem*>(treeWidget->topLevelItem(i));
        if(item && item->getId() == path.path())
        {
            delete item;
            return;
        }
    }
}




IDeviceTreeWidgetItem::IDeviceTreeWidgetItem(const QString& id, QTreeWidget *parent)
    : QTreeWidgetItem(parent)
    , id(id)
{

}
