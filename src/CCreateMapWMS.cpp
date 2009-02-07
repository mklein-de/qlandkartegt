/**********************************************************************************************
    Copyright (C) 2009 Oliver Eichler oliver.eichler@gmx.de

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

#include "CCreateMapWMS.h"
#include "CResources.h"
#include <QtGui>
#include <QtNetwork/QHttp>
#include <QtXml/QDomDocument>

CCreateMapWMS::CCreateMapWMS(QWidget * parent)
: QWidget(parent)
, server(0)
{
    setupUi(this);
    lineServer->setText("http://oberpfalz.geofabrik.de/wms");

    slotSetupLink();
    connect(&CResources::self(), SIGNAL(sigProxyChanged()), this, SLOT(slotSetupLink()));

    connect(pushLoadCapabilities, SIGNAL(clicked()), this, SLOT(slotLoadCapabilities()));
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
    if(WMT_MS_Capabilities.toElement().attribute("version") != "1.1.1"){
        QMessageBox::critical(0,tr("Error..."), tr("Failed to parse check version.\n\n%1\n\n%2").arg(WMT_MS_Capabilities.toElement().attribute("version")), QMessageBox::Abort, QMessageBox::Abort);
        return;
    }

    QDomNode GetMap     = WMT_MS_Capabilities.namedItem("Capability").namedItem("Request").namedItem("GetMap");
    QDomElement format  = GetMap.firstChildElement("Format");

    while(!format.isNull()){
        comboFormat->addItem(format.text());
        format = format.nextSiblingElement("Format");
    }


    QDomNode Layers     = WMT_MS_Capabilities.namedItem("Capability").namedItem("Layer");
    QDomElement srs     = Layers.firstChildElement("SRS");

    while(!srs.isNull()){
        comboProjection->addItem(srs.text());
        srs = srs.nextSiblingElement("SRS");
    }

    QDomNode layer      = Layers.firstChildElement("Layer");
    while(!layer.isNull()){
        QString title = layer.namedItem("Name").toElement().text();
        QListWidgetItem *item = new QListWidgetItem(listLayers);
        item->setText(title);

        layer = layer.nextSiblingElement("Layer");
    }


    frame->setEnabled(true);

}
