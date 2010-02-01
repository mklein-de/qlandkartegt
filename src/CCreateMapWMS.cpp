/**********************************************************************************************
    Copyright (C) 2009 Oliver Eichler oliver.eichler@gmx.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either Version 2 of the License, or
    (at your option) any later Version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/

#include "CCreateMapWMS.h"
#include "CResources.h"
#include "GeoMath.h"
#include "CMapDB.h"
#include "CMainWindow.h"

#include <QtGui>
#include <QtNetwork/QHttp>
#include <QtXml/QDomDocument>
#include <projects.h>

CCreateMapWMS::CCreateMapWMS(QWidget * parent)
: QWidget(parent)
, server(0)
, mapPath("./")
{
    setupUi(this);

    QSettings cfg;
    lineServer->setText(cfg.value("wms/last_server","").toString());
    mapPath = cfg.value("path/maps",mapPath).toString();

    toolFile->setIcon(QIcon(":/icons/iconOpenMap16x16.png"));

    slotSetupLink();
    connect(&CResources::self(), SIGNAL(sigProxyChanged()), this, SLOT(slotSetupLink()));
    connect(pushLoadCapabilities, SIGNAL(clicked()), this, SLOT(slotLoadCapabilities()));
    connect(pushSave, SIGNAL(clicked()), this, SLOT(slotSave()));
    connect(toolFile, SIGNAL(clicked()), this, SLOT(slotSelectFile()));
}


CCreateMapWMS::~CCreateMapWMS()
{
    QSettings cfg;
    cfg.setValue("wms/last_server",lineServer->text());
    cfg.setValue("path/maps",mapPath);
}


void CCreateMapWMS::slotSetupLink()
{
    QString url;
    quint16 port;
    bool enableProxy;

    enableProxy = CResources::self().getHttpProxy(url,port);

    if(server) delete server;
    server = new QHttp(this);
    if(enableProxy)
    {
        server->setProxy(url,port);
    }

    connect(server,SIGNAL(requestStarted(int)),this,SLOT(slotRequestStarted(int)));
    connect(server,SIGNAL(requestFinished(int,bool)),this,SLOT(slotRequestFinished(int,bool)));
}


void CCreateMapWMS::slotLoadCapabilities()
{
    QUrl url(lineServer->text());

    url.addQueryItem("SERVICE","WMS");
    url.addQueryItem("REQUEST","GetCapabilities");

    qDebug() << url;

    server->setHost(url.host());
    server->get(url.toEncoded( ));

    comboFormat->clear();
    listLayers->clear();
    comboProjection->clear();
    lineTitle->clear();
}


void CCreateMapWMS::slotRequestStarted(int )
{
    QApplication::changeOverrideCursor(Qt::WaitCursor);
    frame->setEnabled(false);
}


void CCreateMapWMS::slotRequestFinished(int , bool error)
{
    qDebug() << "slotRequestFinished(int , bool error)";

    QApplication::restoreOverrideCursor();

    if(error)
    {
        QMessageBox::critical(0,tr("Error..."), tr("Failed to query capabilities.\n\n%1\n\n").arg(server->errorString()), QMessageBox::Abort, QMessageBox::Abort);
        return;
    }

    QString asw = server->readAll();
    asw = asw.simplified();

    if(asw.isEmpty())
    {
        return;
    }

    QDomDocument    dom;
    QString         errorMsg;
    int             errorLine;
    int             errorColumn;
    if(!dom.setContent(asw, &errorMsg, &errorLine, &errorColumn))
    {
        QMessageBox::critical(0,tr("Error..."), tr("Failed to parse capabilities.\n\n%1\n\n%2").arg(errorMsg).arg(asw), QMessageBox::Abort, QMessageBox::Abort);
        return;
    }

    qDebug() << dom.toString();

    // Assume more recent version 1.3.0 first
    versionString = "1.3.0";
    QDomNode WMS_Capabilities = dom.namedItem("WMS_Capabilities");

    // If it did not work then try version 1.1.1
    if (WMS_Capabilities.isNull())
    {
        versionString = "1.1.1";
        WMS_Capabilities = dom.namedItem("WMT_MS_Capabilities");
    }

    // Make sure that we guessed the version correctly
    if(versionString != WMS_Capabilities.toElement().attribute("version"))
    {
        QMessageBox::critical(0,tr("Error..."), tr("Failed to check WMS version.\n\nExpected %1, received %2.").arg(
            versionString, WMS_Capabilities.toElement().attribute("version")), QMessageBox::Abort, QMessageBox::Abort);
        return;
    }

    // Find supported tile formats [1.1.1] [1.3.0]
    QDomNode GetMap     = WMS_Capabilities.namedItem("Capability").namedItem("Request").namedItem("GetMap");
    QDomElement format  = GetMap.firstChildElement("Format");

    while(!format.isNull())
    {
        comboFormat->addItem(format.text());
        format = format.nextSiblingElement("Format");
    }

    // Find the resource URL [1.1.1] [1.3.0]
    QDomNode OnlineResource = GetMap.firstChildElement("DCPType").firstChildElement("HTTP").firstChildElement("Get").firstChildElement("OnlineResource");
    urlOnlineResource = OnlineResource.toElement().attribute("xlink:href","");

    // In 1.1.1 we have SRS and in 1.3.0 CRS
    QDomNode layers     = WMS_Capabilities.namedItem("Capability").namedItem("Layer");

    if (versionString == "1.3.0")
    {
        QDomElement CRS = layers.firstChildElement("CRS");
        while(!CRS.isNull())
        {
            comboProjection->addItem(CRS.text());
            CRS = CRS.nextSiblingElement("CRS");
        }
    }
    else
    {
        QDomElement SRS = layers.firstChildElement("SRS");
        while(!SRS.isNull())
        {
            QStringList listSRS = SRS.text().split(" ", QString::SkipEmptyParts);
            foreach(QString strSRS, listSRS)
            {
                comboProjection->addItem(strSRS);
            }
            SRS = SRS.nextSiblingElement("SRS");
        }
    }

    // Find the layers [1.1.1] [1.3.0]
    QDomNode layer      = layers.firstChildElement("Layer");

    while(!layer.isNull())
    {
        QString title = layer.namedItem("Name").toElement().text();
        QListWidgetItem *item = new QListWidgetItem(listLayers);
        item->setText(title);

        layer = layer.nextSiblingElement("Layer");
    }

    lineTitle->setText(layers.namedItem("Title").toElement().text());
    if(labelFile->text().isEmpty())
    {
        QDir path(mapPath);
        labelFile->setText(path.filePath(lineTitle->text() + ".xml"));
    }

    // In 1.1.1 we have LatLonBoundingBox and in 1.3.0 EX_GeographicBoundingBox
    double maxx, maxy, minx, miny;
    if (versionString == "1.3.0")
    {
        QDomElement GeographicBoundingBox = layers.namedItem("EX_GeographicBoundingBox").toElement();

        bool ok;
        maxx = GeographicBoundingBox.namedItem("eastBoundLongitude").toElement().text().toDouble(&ok);
        if (!ok) maxx = 0.0;
        maxy = GeographicBoundingBox.namedItem("northBoundLatitude").toElement().text().toDouble(&ok);
        if (!ok) maxy = 0.0;
        minx = GeographicBoundingBox.namedItem("westBoundLongitude").toElement().text().toDouble(&ok);
        if (!ok) minx = 0.0;
        miny = GeographicBoundingBox.namedItem("southBoundLatitude").toElement().text().toDouble(&ok);
        if (!ok) miny = 0.0;
    }
    else
    {
        QDomElement LatLonBoundingBox = layers.namedItem("LatLonBoundingBox").toElement();

        maxx = LatLonBoundingBox.attribute("maxx","0.0").toDouble();
        maxy = LatLonBoundingBox.attribute("maxy","0.0").toDouble();
        minx = LatLonBoundingBox.attribute("minx","0.0").toDouble();
        miny = LatLonBoundingBox.attribute("miny","0.0").toDouble();
    }

    rectLatLonBoundingBox.setTop(maxy);
    rectLatLonBoundingBox.setLeft(minx);
    rectLatLonBoundingBox.setBottom(miny);
    rectLatLonBoundingBox.setRight(maxx);

    frame->setEnabled(true);
}


void CCreateMapWMS::slotSave()
{
    QDomDocument dom;
    QDomElement  GDAL_WMS = dom.createElement("GDAL_WMS");
    QDomElement  Service  = dom.createElement("Service");
    Service.setAttribute("name", "WMS");

    QDomElement  Title  = dom.createElement("Title");
    Title.appendChild(dom.createTextNode(lineTitle->text()));
    Service.appendChild(Title);

    QDomElement  Version  = dom.createElement("Version");
    Version.appendChild(dom.createTextNode(versionString));
    Service.appendChild(Version);

    QDomElement ServerUrl = dom.createElement("ServerUrl");
    ServerUrl.appendChild(dom.createTextNode(urlOnlineResource));
    Service.appendChild(ServerUrl);

    QDomElement RS;
    if (versionString == "1.3.0") RS = dom.createElement("CRS");
    else RS = dom.createElement("SRS");
    RS.appendChild(dom.createTextNode(comboProjection->currentText()));
    Service.appendChild(RS);

    QDomElement ImageFormat = dom.createElement("ImageFormat");
    ImageFormat.appendChild(dom.createTextNode(comboFormat->currentText()));
    Service.appendChild(ImageFormat);

    QStringList layerlist;
    QList<QListWidgetItem *> items                  = listLayers->selectedItems();
    QList<QListWidgetItem *>::const_iterator item   = items.begin();
    while(item != items.end())
    {
        layerlist << (*item)->text();
        ++item;
    }
    if(layerlist.isEmpty())
    {
        QMessageBox::critical(0,tr("Error..."), tr("You need to select at least one layer."), QMessageBox::Abort, QMessageBox::Abort);
        return;
    }
    QDomElement Layers = dom.createElement("Layers");
    Layers.appendChild(dom.createTextNode(layerlist.join(",")));
    Service.appendChild(Layers);
    GDAL_WMS.appendChild(Service);

    QDomElement DataWindow = dom.createElement("DataWindow");

    double u1 = rectLatLonBoundingBox.left()    * DEG_TO_RAD;
    double v1 = rectLatLonBoundingBox.top()     * DEG_TO_RAD;
    double u2 = rectLatLonBoundingBox.right()   * DEG_TO_RAD;
    double v2 = rectLatLonBoundingBox.bottom()  * DEG_TO_RAD;

    PJ * pjWGS84 = 0, * pjTar = 0;
    pjWGS84 = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");

    qDebug() << QString("+init=%1").arg(comboProjection->currentText()).toLatin1();
    pjTar   = pj_init_plus(QString("+init=%1").arg(comboProjection->currentText()).toLower().toLatin1());

    pj_transform(pjWGS84, pjTar,1,0,&u1,&v1,0);
    pj_transform(pjWGS84, pjTar,1,0,&u2,&v2,0);

    if(pj_is_latlong(pjTar))
    {
        u1 *= RAD_TO_DEG;
        v1 *= RAD_TO_DEG;
        u2 *= RAD_TO_DEG;
        v2 *= RAD_TO_DEG;
    }

    QDomElement UpperLeftX = dom.createElement("UpperLeftX");
    UpperLeftX.appendChild(dom.createTextNode(QString("%1").arg(u1,0,'f')));
    DataWindow.appendChild(UpperLeftX);

    QDomElement UpperLeftY = dom.createElement("UpperLeftY");
    UpperLeftY.appendChild(dom.createTextNode(QString("%1").arg(v1,0,'f')));
    DataWindow.appendChild(UpperLeftY);

    QDomElement LowerRightX = dom.createElement("LowerRightX");
    LowerRightX.appendChild(dom.createTextNode(QString("%1").arg(u2,0,'f')));
    DataWindow.appendChild(LowerRightX);

    QDomElement LowerRightY = dom.createElement("LowerRightY");
    LowerRightY.appendChild(dom.createTextNode(QString("%1").arg(v2,0,'f')));
    DataWindow.appendChild(LowerRightY);

    if(pj_is_latlong(pjTar))
    {
        double  a1 = 0, a2 = 0;
        double sizex, sizey;

        XY north, south, east, west;

        north.v = v1 * DEG_TO_RAD;
        south.v = v2 * DEG_TO_RAD;
        north.u = south.u = (u1+u2)/2 * DEG_TO_RAD;

        east.u = u1 * DEG_TO_RAD;
        west.u = u2 * DEG_TO_RAD;
        east.v = west.v = (v1+v2)/2 * DEG_TO_RAD;

        sizex = parallel_distance(east, west);
        sizey = distance(north, south, a1, a2);

        QDomElement SizeX = dom.createElement("SizeX");
        SizeX.appendChild(dom.createTextNode(QString("%1").arg(sizex,0,'f')));
        DataWindow.appendChild(SizeX);

        QDomElement SizeY = dom.createElement("SizeY");
        SizeY.appendChild(dom.createTextNode(QString("%1").arg(sizey,0,'f')));
        DataWindow.appendChild(SizeY);
    }
    else
    {
        QDomElement SizeX = dom.createElement("SizeX");
        SizeX.appendChild(dom.createTextNode(QString("%1").arg(u2 - u1,0,'f')));
        DataWindow.appendChild(SizeX);

        QDomElement SizeY = dom.createElement("SizeY");
        SizeY.appendChild(dom.createTextNode(QString("%1").arg(v1 - v2,0,'f')));
        DataWindow.appendChild(SizeY);
    }
    pj_free(pjWGS84);
    pj_free(pjTar);

    GDAL_WMS.appendChild(DataWindow);

    QDomElement Projection = dom.createElement("Projection");
    Projection.appendChild(dom.createTextNode(comboProjection->currentText()));
    GDAL_WMS.appendChild(Projection);

    QDomElement BandsCount = dom.createElement("BandsCount");
    if(comboFormat->currentText().contains("png"))
    {
        BandsCount.appendChild(dom.createTextNode("4"));
    }
    else
    {
        BandsCount.appendChild(dom.createTextNode("3"));
    }
    GDAL_WMS.appendChild(BandsCount);

    QDomElement Timeout = dom.createElement("Timeout");
    Timeout.appendChild(dom.createTextNode("20"));
    GDAL_WMS.appendChild(Timeout);

    QDomElement BlockSizeX = dom.createElement("BlockSizeX");
    BlockSizeX.appendChild(dom.createTextNode("512"));
    GDAL_WMS.appendChild(BlockSizeX);

    QDomElement BlockSizeY = dom.createElement("BlockSizeY");
    BlockSizeY.appendChild(dom.createTextNode("512"));
    GDAL_WMS.appendChild(BlockSizeY);

    dom.appendChild(GDAL_WMS);

    QString filename = labelFile->text();

    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    file.write(dom.toString().toLocal8Bit());
    file.close();

    qDebug() << dom.toString();

    CMapDB::self().openMap(filename, false, *theMainWindow->getCanvas());
}


void CCreateMapWMS::slotSelectFile()
{
    QString filename;

    filename = QFileDialog::getSaveFileName(0,tr("Define GDAL WMS definition file..."), mapPath,"GDAL WMS definition (*.xml)", 0, QFileDialog::DontUseNativeDialog);
    if(filename.isEmpty()) return;
    mapPath = QFileInfo(filename).path();

    QFileInfo fi(filename);
    if(fi.suffix() != "xml")
    {
        filename += ".xml";
    }

    mapPath = fi.path();
    labelFile->setText(filename);
}
