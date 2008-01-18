/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  FAX:         +49-941-83055-79

  File:        CSearchDB.cpp

  Module:

  Description:

  Created:     01/17/2008

  (C) 2008


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
}

CSearchDB::result_t * CSearchDB::getResultByKey(const QString& key)
{
    if(!results.contains(key)) return 0;

    return &results[key];
}
