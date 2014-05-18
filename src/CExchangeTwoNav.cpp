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
    if(!path.path().startsWith("/org/freedesktop/UDisks2/block_devices/"))
    {
        return;
    }
    QDBusInterface * blockIface = new QDBusInterface("org.freedesktop.UDisks2",
                                         path.path(),
                                         "org.freedesktop.UDisks2.Block",
                                         QDBusConnection::systemBus(),
                                         this);
    Q_ASSERT(blockIface);

    QDBusObjectPath drive_object = blockIface->property("Drive").value<QDBusObjectPath>();
    qDebug() << drive_object.path();

    QDBusInterface * driveIface = new QDBusInterface("org.freedesktop.UDisks2",
                                         drive_object.path(),
                                         "org.freedesktop.UDisks2.Drive",
                                         QDBusConnection::systemBus(),
                                         this);
    Q_ASSERT(driveIface);
    QString vendor = driveIface->property("Vendor").toString();
    qDebug() << vendor;

    if(vendor.toUpper() != "GENERAL")
    {
        return;
    }

    quint64 size = blockIface->property("Size").toULongLong();
    if(size == 0)
    {
        return;
    }


    CDeviceTreeWidgetItem * item = new CDeviceTreeWidgetItem(path.path(), treeWidget);
    item->setText(0, "TwoNav " + driveIface->property("Model").toString());
    item->setIcon(0, QIcon("://icons/iconDeviceTwoNav16x16.png"));
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
        CDeviceTreeWidgetItem * item = dynamic_cast<CDeviceTreeWidgetItem*>(treeWidget->topLevelItem(i));
        if(item && item->getId() == path.path())
        {
            delete item;
        }
    }

}

//#ifdef Q_OS_LINUX
//void CExchangeTwoNav::slotQueryDevices()
//{
//    QDBusMessage call = QDBusMessage::createMethodCall(UDISK_SERVICE, UDISK_PATH, UDISK_INTERFACE, "EnumerateDevices");


//    QDBusPendingReply< QList<QDBusObjectPath> > reply = QDBusConnection::systemBus().asyncCall(call);
//    reply.waitForFinished();
//    foreach(const QDBusObjectPath& path, reply.value())
//    {
//        slotAddDevice(path);
//    }
//}

//void CExchangeTwoNav::slotAddDevice(const QDBusObjectPath& path)
//{
//    qDebug() << "add device:" << path.path();

//    QDBusMessage call = QDBusMessage::createMethodCall(UDISK_SERVICE, path.path(), "org.freedesktop.DBus.Properties", "GetAll");

//    QList<QVariant> args;
//    args.append("org.freedesktop.UDisks.Device");
//    call.setArguments(args);

//    QDBusPendingReply<QVariantMap> reply = QDBusConnection::systemBus().asyncCall(call);
//    reply.waitForFinished();
//    QVariantMap map = reply.value();

//    QString deviceLabel = map["IdLabel"].toString();
//    if(deviceLabel.isEmpty())
//    {
//        deviceLabel = map["IdUuid"].toString();
//    }

//    QString driverModel = map["DriveModel"].toString().toUpper();
//    if(driverModel.contains("GARMIN") && !deviceLabel.isEmpty())
//    {
//        qDebug() << "!!!!!!!!!!!!!It's a Garmin!!!!!!!!!!!!!";
//        qDebug() << "Device node is" << map["DeviceFilePresentation"];
//        if(map["DeviceIsMounted"].toBool())
//        {
//            qDebug() << "Device is mounted to:" << map["DeviceMountPaths"].toString();
//        }
//        else
//        {
//            qDebug() << "Device is not mounted";
//        }

//        CDeviceTreeWidgetItem * item = new CDeviceTreeWidgetItem(path.path(), treeWidget);
//        item->setText(0, driverModel);
//        item->setIcon(0, QIcon("://icons/iconDeviceGarmin16x16.png"));
//    }

//}

//void CExchangeTwoNav::slotRemoveDevice(const QDBusObjectPath& path)
//{
//    qDebug() << "remove device:" << path.path();
//    const int N = treeWidget->topLevelItemCount();
//    for(int i = 0; i<N; i++)
//    {
//        CDeviceTreeWidgetItem * item = dynamic_cast<CDeviceTreeWidgetItem*>(treeWidget->topLevelItem(i));
//        if(item && item->getId() == path.path())
//        {
//            delete item;
//        }
//    }
//}

//void CExchangeTwoNav::slotChangeDevice(const QDBusObjectPath& path)
//{
//    qDebug() << "change device:" << path.path();
//}
//#endif //Q_OS_LINUX

