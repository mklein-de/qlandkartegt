/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/

#include "CSearchDB.h"
#include "CSearchToolWidget.h"
#include "CResources.h"
#include "CMainWindow.h"
#include "CCanvas.h"

#include <QtGui>
#include <QtNetwork/QHttp>

CSearchDB * CSearchDB::m_self;

static const char google_api_key[] = "ABQIAAAAPztEvITCpkvDNrq-hFRvThQNZ4aRbgDVTL9C0r5u06RhgW2EtRR8yuKglxlHgpZfC5_TdLXlJvIWgA";

CSearchDB::CSearchDB(QToolBox * tb, QObject * parent)
    : IDB(tb,parent)
    , google(0)
{
    m_self      = this;
    toolview    = new CSearchToolWidget(tb);

    slotSetupLink();
    connect(&CResources::self(), SIGNAL(sigProxyChanged()), this, SLOT(slotSetupLink()));

}

CSearchDB::~CSearchDB()
{

}

void CSearchDB::search(const QString& str)
{
    QUrl url;

    if(google == 0) return;

    emit sigStatus("");

    tmpResult.query = str;

    url.setPath("/maps/geo");
    url.addQueryItem("q",str);
    url.addQueryItem("output","csv");
    url.addQueryItem("key",google_api_key);
    google->get(url.toEncoded( ));

}

void CSearchDB::slotSetupLink()
{
    QString url;
    quint16 port;
    bool enableProxy;

    enableProxy = CResources::self().getHttpProxy(url,port);

    if(google) delete google;
    google = new QHttp(this);
    if(enableProxy){
        google->setProxy(url,port);
    }
    google->setHost("maps.google.com");
    connect(google,SIGNAL(requestStarted(int)),this,SLOT(slotRequestStarted(int)));
    connect(google,SIGNAL(requestFinished(int,bool)),this,SLOT(slotRequestFinished(int,bool)));
}

void CSearchDB::slotRequestStarted(int )
{

}

void CSearchDB::slotRequestFinished(int , bool error)
{

    if(error){
        emit sigStatus(google->errorString());

    }

    QString asw = google->readAll();
    asw = asw.simplified();

    if(asw.isEmpty()){
        emit sigFinished();
        return;
    }

    QStringList values = asw.split(",");

    if(values.count() != 4){
        emit sigStatus(tr("Bad number of return paramters"));
    }
    else if(values[0] == "200"){

        tmpResult.lat = values[2].toDouble();
        tmpResult.lon = values[3].toDouble();
        results[tmpResult.query] = tmpResult;

        theMainWindow->getCanvas()->move(tmpResult.lon, tmpResult.lat);

        emit sigStatus(tr("Success."));
    }
    else if(values[0] == "500"){
        emit sigStatus(tr("Failed. Reason unknown."));
    }
    else if(values[0] == "601"){
        emit sigStatus(tr("Failed. Missing query string."));
    }
    else if(values[0] == "602"){
        emit sigStatus(tr("Failed. No location found."));
    }
    else if(values[0] == "603"){
        emit sigStatus(tr("Failed. No location because of legal matters."));
    }
    else if(values[0] == "610"){
        emit sigStatus(tr("Failed. Bad API key."));
    }
    else{
        emit sigStatus(asw);
    }

    emit sigFinished();
    emit sigChanged();
}

CSearchDB::result_t * CSearchDB::getResultByKey(const QString& key)
{
    if(!results.contains(key)) return 0;

    return &results[key];
}
