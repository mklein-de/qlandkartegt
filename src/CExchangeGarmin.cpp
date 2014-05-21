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
#include <QtXml>

CGarminTreeWidgetItem::CGarminTreeWidgetItem(const QString& id, QTreeWidget *parent)
    : IDeviceTreeWidgetItem(id,parent)
{
    setIcon(0, QIcon("://icons/iconDeviceGarmin16x16.png"));
}

void CGarminTreeWidgetItem::readDevice()
{
    pathGpx.clear();
    pathSpoiler.clear();
    pathJpeg.clear();
    pathAdventure.clear();

    QDir dir(mountPoint);
    if(dir.exists("Garmin/GarminDevice.xml"))
    {
        readDeviceXml(dir.absoluteFilePath("Garmin/GarminDevice.xml"));
    }
    else
    {
        QTreeWidgetItem * item = new QTreeWidgetItem(this);
        item->setText(0,QObject::tr("No 'Garmin/GarminDevice.xml' found"));
        item->setIcon(0,QIcon("://icons/iconWarning16x16.png"));
        return;
    }



}

void CGarminTreeWidgetItem::readDeviceXml(const QString& filename)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    QDomDocument dom;
    dom.setContent(&file);

    QDomElement device              = dom.firstChildElement("Device");
    QDomElement MassStorageMode     = device.firstChildElement("MassStorageMode");
    const QDomNodeList& DataTypes   = MassStorageMode.elementsByTagName("DataType");

    for(int i = 0; i < DataTypes.size(); i++)
    {
        QDomNode dataType       = DataTypes.at(i);
        QDomElement Name        = dataType.firstChildElement("Name");
        QDomElement File        = dataType.firstChildElement("File");
        QDomElement Location    = File.firstChildElement("Location");
        QDomElement Path        = Location.firstChildElement("Path");

        qDebug() << Name.text().simplified() << Path.text().simplified();

        QString name = Name.text().simplified();

        if(name == "GPSData")
        {
            pathGpx = Path.text().simplified();
        }
        else if(name == "UserDataSync")
        {
            pathGpx = Path.text().simplified();
        }
        else if(name == "GeotaggedPhotos")
        {
            pathJpeg = Path.text().simplified();
        }
        else if(name == "GeocachePhotos")
        {
            pathSpoiler = Path.text().simplified();
        }
        else if(name == "Adventures")
        {
            pathAdventure = Path.text().simplified();
        }
    }
}

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
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

