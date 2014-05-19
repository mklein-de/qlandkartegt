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

IExchange::IExchange(QTreeWidget *treeWidget, QObject *parent)
    : QObject(parent)
    , treeWidget(treeWidget)
{

}

IExchange::~IExchange()
{

}

QString IExchange::checkForDevice(const QDBusObjectPath& path, const QString& strVendor)
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
    QString vendor = driveIface->property("Vendor").toString();

    // vendor must match
    if(vendor.toUpper() != strVendor)
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


IDeviceTreeWidgetItem::IDeviceTreeWidgetItem(const QString& id, QTreeWidget *parent)
    : QTreeWidgetItem(parent)
    , id(id)
{

}
