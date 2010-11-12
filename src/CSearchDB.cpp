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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/

#include "CSearchDB.h"
#include "CSearchToolWidget.h"
#include "CResources.h"
#include "CMainWindow.h"
#include "CCanvas.h"
#include "CMapDB.h"
#include "IMap.h"

#include <QtGui>
#include <QHttp>
#include <QtXml>

CSearchDB * CSearchDB::m_self;

static const char google_api_key[] = "ABQIAAAAPztEvITCpkvDNrq-hFRvThQNZ4aRbgDVTL9C0r5u06RhgW2EtRR8yuKglxlHgpZfC5_TdLXlJvIWgA";

CSearchDB::CSearchDB(QTabWidget * tb, QObject * parent)
: IDB(tb,parent)
, google(0)
, ors(0)
, tmpResult(0)
{
    m_self      = this;
    toolview    = new CSearchToolWidget(tb);

    slotSetupLink();
    connect(&CResources::self(), SIGNAL(sigProxyChanged()), this, SLOT(slotSetupLink()));

}


CSearchDB::~CSearchDB()
{

}


void CSearchDB::clear()
{
    results.clear();
    emit sigChanged();
}


void CSearchDB::search(const QString& str, hosts_t host)
{

    if(host == eGoogle)
    {
        startGoogle(str);
    }
    if(host == eOpenRouteService)
    {
        startOpenRouteService(str);
    }
    else
    {
        emit sigStatus(tr("Unknown host."));
        emit sigFinished();
        emit sigChanged();
    }
}


void CSearchDB::startGoogle(const QString& str)
{

    QUrl url;

    if(google == 0) return;

    emit sigStatus(tr("start searching..."));

    tmpResult.setName(str);

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
    if(enableProxy)
    {
        google->setProxy(url,port);
    }
    google->setHost("maps.google.com");
    connect(google,SIGNAL(requestFinished(int,bool)),this,SLOT(slotRequestFinishedGoogle(int,bool)));


    if(ors) delete ors;
    ors = new QHttp(this);
    if(enableProxy)
    {
        ors->setProxy(url,port);
    }
    ors->setHost("openls.geog.uni-heidelberg.de");
    connect(ors,SIGNAL(requestFinished(int,bool)),this,SLOT(slotRequestFinishedOpenRouteService(int,bool)));
}


void CSearchDB::slotRequestFinishedGoogle(int , bool error)
{
    QApplication::restoreOverrideCursor();

    if(error)
    {
        emit sigStatus(google->errorString());

    }

    QString asw = google->readAll();
    asw = asw.simplified();

    if(asw.isEmpty())
    {
        emit sigFinished();
        return;
    }

    QStringList values = asw.split(",");

    if(values.count() != 4)
    {
        emit sigStatus(tr("Bad number of return paramters"));
    }
    else if(values[0] == "200")
    {

        tmpResult.lat = values[2].toDouble();
        tmpResult.lon = values[3].toDouble();

        CSearch * item = new CSearch(this);
        item->lon   = tmpResult.lon;
        item->lat   = tmpResult.lat;
        item->setName(tmpResult.getName());

        results[item->getKey()] = item;

        theMainWindow->getCanvas()->move(item->lon, item->lat);

        emit sigStatus(tr("Success."));
    }
    else if(values[0] == "500")
    {
        emit sigStatus(tr("Failed. Reason unknown."));
    }
    else if(values[0] == "601")
    {
        emit sigStatus(tr("Failed. Missing query string."));
    }
    else if(values[0] == "602")
    {
        emit sigStatus(tr("Failed. No location found."));
    }
    else if(values[0] == "603")
    {
        emit sigStatus(tr("Failed. No location because of legal matters."));
    }
    else if(values[0] == "610")
    {
        emit sigStatus(tr("Failed. Bad API key."));
    }
    else
    {
        emit sigStatus(asw);
    }

    emit sigFinished();
    emit sigChanged();
}


CSearch * CSearchDB::getResultByKey(const QString& key)
{
    if(!results.contains(key)) return 0;

    return results[key];
}


void CSearchDB::draw(QPainter& p, const QRect& rect, bool& needsRedraw)
{
    IMap& map = CMapDB::self().getMap();

    QMap<QString,CSearch*>::const_iterator result = results.begin();
    while(result != results.end())
    {
        double u = (*result)->lon * DEG_TO_RAD;
        double v = (*result)->lat * DEG_TO_RAD;
        map.convertRad2Pt(u,v);

        if(rect.contains(QPoint(u,v)))
        {
            p.drawPixmap(u-8 , v-8, QPixmap(":/icons/iconBullseye16x16.png"));
            CCanvas::drawText((*result)->getName(), p, QPoint(u, v - 10));
        }

        ++result;
    }
}


void CSearchDB::delResults(const QStringList& keys)
{

    QString key;
    foreach(key, keys)
    {
        results.remove(key);
    }

    emit sigChanged();

}


void CSearchDB::add(const QString& label, double lon, double lat)
{
    CSearch * item = new CSearch(this);
    item->lon   = lon * RAD_TO_DEG;
    item->lat   = lat * RAD_TO_DEG;
    item->setName(label);

    results[item->getKey()] = item;

    emit sigChanged();
}

void CSearchDB::selSearchByKey(const QString& key)
{
    CSearchToolWidget * t = qobject_cast<CSearchToolWidget*>(toolview);
    if(t)
    {
        t->selSearchByKey(key);
        gainFocus();
    }
}

const QString CSearchDB::xls_ns = "http://www.opengis.net/xls";
const QString CSearchDB::sch_ns = "http://www.ascc.net/xml/schematron";
const QString CSearchDB::gml_ns = "http://www.opengis.net/gml";
const QString CSearchDB::xlink_ns = "http://www.w3.org/1999/xlink";
const QString CSearchDB::xsi_ns = "http://www.w3.org/2001/XMLSchema-instance";
const QString CSearchDB::schemaLocation = "http://www.opengis.net/xls http://schemas.opengis.net/ols/1.1.0/LocationUtilityService.xsd";

void CSearchDB::startOpenRouteService(const QString& str)
{

    if(ors == 0) return;

    emit sigStatus(tr("start searching..."));

    QDomDocument xml;
    QDomElement root = xml.createElement("xls:XLS");
    xml.appendChild(root);

    root.setAttribute("xmlns:xls",xls_ns);
    root.setAttribute("xmlns:sch",sch_ns);
    root.setAttribute("xmlns:gml",gml_ns);
    root.setAttribute("xmlns:xlink",xlink_ns);
    root.setAttribute("xmlns:xsi",xsi_ns);
    root.setAttribute("xsi:schemaLocation",schemaLocation);
    root.setAttribute("version","1.1");
    //root.setAttribute("xls:lang", comboLanguage->itemData(comboLanguage->currentIndex()).toString());

    QDomElement requestHeader = xml.createElement("xls:RequestHeader");
    root.appendChild(requestHeader);

    QDomElement Request = xml.createElement("xls:Request");
    root.appendChild(Request);

    Request.setAttribute("methodName", "GeocodeRequest");
    Request.setAttribute("requestID", "12345");
    Request.setAttribute("version", "1.1");

    QDomElement GeocodeRequest = xml.createElement("xls:GeocodeRequest");
    Request.appendChild(GeocodeRequest);

    QDomElement Address = xml.createElement("xls:Address");
    GeocodeRequest.appendChild(Address);

    Address.setAttribute("countryCode", "DE");

    QDomElement freeFormAddress = xml.createElement("xls:freeFormAddress");
    Address.appendChild(freeFormAddress);

    QDomText _freeFormAddress_ = xml.createTextNode(str);
    freeFormAddress.appendChild(_freeFormAddress_);

//    qDebug() << xml.toString();

    QByteArray array;
    QTextStream out(&array, QIODevice::WriteOnly);
    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << xml.toString() << endl;

    QUrl url;
    url.setPath("/qlandkarte/geocode");

    ors->post(url.toEncoded(), array);
}

void CSearchDB::slotRequestFinishedOpenRouteService(int , bool error)
{
    QDomDocument xml;
    QString status = tr("finished");

    if(error)
    {
        QMessageBox::warning(0,tr("Failed..."), tr("Bad response from server:\n%1").arg(ors->errorString()), QMessageBox::Abort);
        emit sigStatus(tr("failed"));
        emit sigFinished();
        emit sigChanged();
        return;
    }

    QByteArray res = ors->readAll();
    if(res.isEmpty())
    {
        return;
    }

    xml.setContent(res);

    qDebug() << xml.toString();

    QDomElement root = xml.documentElement();
    QDomNodeList Results = root.elementsByTagName("xls:GeocodedAddress");
    const qint32 N = Results.size();

    if(N)
    {
        for(int i = 0; i < N; i++)
        {
            QString street, country, municipal, ccode;

            QDomElement Result = Results.item(i).toElement();
            QDomElement Point = Result.firstChildElement("gml:Point").toElement();
            QString strpos = Point.firstChildElement("gml:pos").toElement().text();

            QDomElement Address = Result.firstChildElement("xls:Address").toElement();
            ccode = Address.attribute("countryCode","");

            QDomElement StreetAddress = Address.firstChildElement("xls:StreetAddress").toElement();
            QDomElement Street = StreetAddress.firstChildElement("xls:Street").toElement();
            street = Street.attribute("officialName","");

            QDomNodeList places = Result.elementsByTagName("xls:Place");
            const qint32 M = places.size();
            for(int j = 0; j < M; j++)
            {
                QDomElement place = places.item(j).toElement();
                QString type = place.attribute("type","");

                if(type == "CountrySubdivision")
                {
                    country = place.text();
                }
                else if(type == "Municipality")
                {
                    municipal = place.text();
                }
            }

            CSearch * item = new CSearch(this);
            item->lon   = strpos.section(" ", 0, 0).toFloat();
            item->lat   = strpos.section(" ", 1, 1).toFloat();
            item->setName(street + ", " + municipal + ", " + country + ", " + ccode);
            results[item->getKey()] = item;
            theMainWindow->getCanvas()->move(item->lon, item->lat);


        }
    }
    else
    {
        status = tr("no result");
    }



    emit sigStatus(status);
    emit sigFinished();
    emit sigChanged();

}
