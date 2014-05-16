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

#define UDISK_SERVICE       "org.freedesktop.UDisks"
#define UDISK_PATH          "/org/freedesktop/UDisks"
#define UDISK_INTERFACE     "org.freedesktop.UDisks"

CExchangeGarmin::CExchangeGarmin(QTreeWidget * treeWidget, QObject * parent)
    : IExchange(treeWidget,parent)
{
#ifdef Q_OS_LINUX
    QDBusConnection::systemBus().connect(UDISK_SERVICE, UDISK_PATH, UDISK_INTERFACE, "DeviceAdded", this, SLOT(slotAddDevice(QDBusObjectPath)));
    QDBusConnection::systemBus().connect(UDISK_SERVICE, UDISK_PATH, UDISK_INTERFACE, "DeviceRemoved", this, SLOT(slotRemoveDevice(QDBusObjectPath)));
    QDBusConnection::systemBus().connect(UDISK_SERVICE, UDISK_PATH, UDISK_INTERFACE, "DeviceChanged", this, SLOT(slotChangeDevice(QDBusObjectPath)));

    QTimer::singleShot(1000, this, SLOT(slotQueryDevices()));
#endif // Q_OS_LINUX
}

CExchangeGarmin::~CExchangeGarmin()
{

}

#ifdef Q_OS_LINUX
void CExchangeGarmin::slotQueryDevices()
{
    QDBusMessage call = QDBusMessage::createMethodCall(UDISK_SERVICE, UDISK_PATH, UDISK_INTERFACE, "EnumerateDevices");


    QDBusPendingReply< QList<QDBusObjectPath> > reply = QDBusConnection::systemBus().asyncCall(call);
    reply.waitForFinished();
    foreach(const QDBusObjectPath& path, reply.value())
    {
        slotAddDevice(path);
    }
}

void CExchangeGarmin::slotAddDevice(const QDBusObjectPath& path)
{
    qDebug() << "add device:" << path.path();

    QDBusMessage call = QDBusMessage::createMethodCall(UDISK_SERVICE, path.path(), "org.freedesktop.DBus.Properties", "GetAll");

    QList<QVariant> args;
    args.append("org.freedesktop.UDisks.Device");
    call.setArguments(args);

    QDBusPendingReply<QVariantMap> reply = QDBusConnection::systemBus().asyncCall(call);
    reply.waitForFinished();
    QVariantMap map = reply.value();

    QString deviceLabel = map["IdLabel"].toString();
    if(deviceLabel.isEmpty())
    {
        deviceLabel = map["IdUuid"].toString();
    }

    QString driverModel = map["DriveModel"].toString().toUpper();
    if(driverModel.contains("GARMIN") && !deviceLabel.isEmpty())
    {
        qDebug() << "!!!!!!!!!!!!!It's a Garmin!!!!!!!!!!!!!";
        qDebug() << "Device node is" << map["DeviceFilePresentation"];
        if(map["DeviceIsMounted"].toBool())
        {
            qDebug() << "Device is mounted to:" << map["DeviceMountPaths"].toString();
        }
        else
        {
            qDebug() << "Device is not mounted";
        }

        CDeviceTreeWidgetItem * item = new CDeviceTreeWidgetItem(path.path(), treeWidget);
        item->setText(0, driverModel);
        item->setIcon(0, QIcon("://icons/iconDeviceGarmin16x16.png"));
    }

}

void CExchangeGarmin::slotRemoveDevice(const QDBusObjectPath& path)
{
    qDebug() << "remove device:" << path.path();
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

void CExchangeGarmin::slotChangeDevice(const QDBusObjectPath& path)
{
    qDebug() << "change device:" << path.path();
}
#endif //Q_OS_LINUX

