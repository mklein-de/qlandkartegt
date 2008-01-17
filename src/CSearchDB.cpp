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
//     lineInput->setEnabled(false);
}

void CSearchDB::slotRequestFinished(int , bool error)
{
//     lineInput->setEnabled(true);
    if(error){
        emit sigStatus(google->errorString());

    }

    QString asw = google->readAll();
    asw = asw.simplified();

    if(asw.isEmpty()) return;

//     labelStatus->clear();

//     qDebug() << asw;
    QStringList values = asw.split(",");

    if(values.count() != 4){
        emit sigStatus(tr("Bad number of return paramters"));
    }
    else if(values[0] == "200"){
        double longitude = values[3].toDouble();
        double latitude  = values[2].toDouble();
//         gpResources->canvas().move(longitude,latitude,lineInput->currentText());

//         if(listQuery->findItems(lineInput->currentText(),Qt::MatchExactly).isEmpty()){
//             QListWidgetItem *item = new QListWidgetItem(lineInput->currentText(),0);
//             item->setData(Qt::UserRole,longitude);
//             item->setData(Qt::UserRole + 1,latitude);
//             listQuery->insertItem(0,item);
//         }
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
}

