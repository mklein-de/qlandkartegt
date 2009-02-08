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
#include <QtGui>
#include <QtNetwork/QHttp>
#include <QtXml/QDomDocument>
#include <projects.h>

CCreateMapWMS::CCreateMapWMS(QWidget * parent)
: QWidget(parent)
, server(0)
{
    setupUi(this);
//     lineServer->setText("http://oberpfalz.geofabrik.de/wms");
    lineServer->setText("http://geodaten.bayern.de/ogc/ogc_atkis_test.cgi");


    slotSetupLink();
    connect(&CResources::self(), SIGNAL(sigProxyChanged()), this, SLOT(slotSetupLink()));
    connect(pushLoadCapabilities, SIGNAL(clicked()), this, SLOT(slotLoadCapabilities()));
    connect(pushSave, SIGNAL(clicked()), this, SLOT(slotSave()));
}

CCreateMapWMS::~CCreateMapWMS()
{

}

void CCreateMapWMS::slotSetupLink()
{
    QString url;
    quint16 port;
    bool enableProxy;

    enableProxy = CResources::self().getHttpProxy(url,port);

    if(server) delete server;
    server = new QHttp(this);
    if(enableProxy) {
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
    labelTitle->clear();
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

    if(error) {
        QMessageBox::critical(0,tr("Error..."), tr("Failed to query capabilities.\n\n%1\n\n").arg(server->errorString()), QMessageBox::Abort, QMessageBox::Abort);
        return;
    }

    QString asw = server->readAll();
    asw = asw.simplified();

    if(asw.isEmpty()) {
        return;
    }

    QDomDocument    dom;
    QString         errorMsg;
    int             errorLine;
    int             errorColumn;
    if(!dom.setContent(asw, &errorMsg, &errorLine, &errorColumn)){
        QMessageBox::critical(0,tr("Error..."), tr("Failed to parse capabilities.\n\n%1\n\n%2").arg(errorMsg).arg(asw), QMessageBox::Abort, QMessageBox::Abort);
        return;
    }

    qDebug() << dom.toString();

    QDomNode WMT_MS_Capabilities = dom.namedItem("WMT_MS_Capabilities");
//     if(WMT_MS_Capabilities.toElement().attribute("version") != "1.1.1"){
//         QMessageBox::critical(0,tr("Error..."), tr("Failed to check Version.\n\n%1").arg(WMT_MS_Capabilities.toElement().attribute("Version")), QMessageBox::Abort, QMessageBox::Abort);
//         return;
//     }

    QDomNode GetMap     = WMT_MS_Capabilities.namedItem("Capability").namedItem("Request").namedItem("GetMap");
    QDomElement format  = GetMap.firstChildElement("Format");

    while(!format.isNull()){
        comboFormat->addItem(format.text());
        format = format.nextSiblingElement("Format");
    }

    QDomNode OnlineResource = GetMap.firstChildElement("DCPType").firstChildElement("HTTP").firstChildElement("Get").firstChildElement("OnlineResource");
    urlOnlineResource = OnlineResource.toElement().attribute("xlink:href","");

    QDomNode layers     = WMT_MS_Capabilities.namedItem("Capability").namedItem("Layer");
    QDomElement srs     = layers.firstChildElement("SRS");

    while(!srs.isNull()){
        QString strSRS      = srs.text();
        QStringList listSRS = strSRS.split(" ", QString::SkipEmptyParts);
        foreach(strSRS, listSRS){
            comboProjection->addItem(strSRS);
        }
        srs = srs.nextSiblingElement("SRS");
    }

    QDomNode layer      = layers.firstChildElement("Layer");

    while(!layer.isNull()){
        QString title = layer.namedItem("Name").toElement().text();
        QListWidgetItem *item = new QListWidgetItem(listLayers);
        item->setText(title);

        layer = layer.nextSiblingElement("Layer");
    }

    labelTitle->setText(layers.namedItem("Title").toElement().text());
    if(labelFile->text().isEmpty()){
        labelFile->setText(labelTitle->text() + ".xml");
    }

    QDomElement LatLonBoundingBox = layers.namedItem("LatLonBoundingBox").toElement();
    double maxx = LatLonBoundingBox.attribute("maxx","0.0").toDouble();
    double maxy = LatLonBoundingBox.attribute("maxy","0.0").toDouble();
    double minx = LatLonBoundingBox.attribute("minx","0.0").toDouble();
    double miny = LatLonBoundingBox.attribute("miny","0.0").toDouble();

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

    QDomElement  Version  = dom.createElement("Version");
    Version.appendChild(dom.createTextNode("1.1.1"));
    Service.appendChild(Version);

    QDomElement ServerUrl = dom.createElement("ServerUrl");
    ServerUrl.appendChild(dom.createTextNode(urlOnlineResource));
    Service.appendChild(ServerUrl);

    QDomElement SRS       = dom.createElement("SRS");
    SRS.appendChild(dom.createTextNode(comboProjection->currentText()));
    Service.appendChild(SRS);

    QDomElement ImageFormat = dom.createElement("ImageFormat");
    ImageFormat.appendChild(dom.createTextNode(comboFormat->currentText()));
    Service.appendChild(ImageFormat);

    QStringList layerlist;
    QList<QListWidgetItem *> items                  = listLayers->selectedItems();
    QList<QListWidgetItem *>::const_iterator item   = items.begin();
    while(item != items.end()){
        layerlist << (*item)->text();
        ++item;
    }
    if(layerlist.isEmpty()){
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

    if(pj_is_latlong(pjTar)){
        u1 *= RAD_TO_DEG;
        v1 *= RAD_TO_DEG;
        u2 *= RAD_TO_DEG;
        v2 *= RAD_TO_DEG;
    }

    pj_free(pjWGS84);
    pj_free(pjTar);

    QDomElement UpperLeftX = dom.createElement("UpperLeftX");
    UpperLeftX.appendChild(dom.createTextNode(QString("%1").arg(u1)));
    DataWindow.appendChild(UpperLeftX);

    QDomElement UpperLeftY = dom.createElement("UpperLeftY");
    UpperLeftY.appendChild(dom.createTextNode(QString("%1").arg(v1)));
    DataWindow.appendChild(UpperLeftY);

    QDomElement LowerRightX = dom.createElement("LowerRightX");
    LowerRightX.appendChild(dom.createTextNode(QString("%1").arg(u2)));
    DataWindow.appendChild(LowerRightX);

    QDomElement LowerRightY = dom.createElement("LowerRightY");
    LowerRightY.appendChild(dom.createTextNode(QString("%1").arg(v2)));
    DataWindow.appendChild(LowerRightY);

    QDomElement SizeX = dom.createElement("SizeX");
    SizeX.appendChild(dom.createTextNode(QString("%1").arg(u2 - u1)));
    DataWindow.appendChild(SizeX);

    QDomElement SizeY = dom.createElement("SizeY");
    SizeY.appendChild(dom.createTextNode(QString("%1").arg(v2 - v1)));
    DataWindow.appendChild(SizeY);

    GDAL_WMS.appendChild(DataWindow);

    QDomElement Projection = dom.createElement("Projection");
    Projection.appendChild(dom.createTextNode(comboProjection->currentText()));
    GDAL_WMS.appendChild(Projection);

    QDomElement BandsCount = dom.createElement("BandsCount");
    BandsCount.appendChild(dom.createTextNode("3"));
    GDAL_WMS.appendChild(BandsCount);


    dom.appendChild(GDAL_WMS);

    QFile file("test.xml");
    file.open(QIODevice::WriteOnly);
    file.write(dom.toString().toLocal8Bit());
    file.close();

    qDebug() << dom.toString();
}
