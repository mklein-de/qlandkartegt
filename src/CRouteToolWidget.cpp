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
#include "CRouteToolWidget.h"
#include "CRoute.h"
#include "CRouteDB.h"
#include "IUnit.h"
#include "CMapDB.h"
#include "CDlgEditRoute.h"
#include "CResources.h"
#include "COverlayDB.h"
#include "COverlayDistance.h"
#include "CTrackDB.h"
#include "CTrack.h"
#include "CDlgConvertToTrack.h"
#include "CMegaMenu.h"
#include "version.h"


#include <QtGui>
#include <QtXml>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkProxy>

#define N_LINES 3

CRouteToolWidget::sortmode_e CRouteToolWidget::sortmode = CRouteToolWidget::eSortByName;

CRouteToolWidget::CRouteToolWidget(QTabWidget * parent)
: QWidget(parent)
, originator(false)
{
    setupUi(this);
    setObjectName("Routes");
    parent->addTab(this,QIcon(":/icons/iconRoute16x16.png"),"");
    parent->setTabToolTip(parent->indexOf(this), tr("Routes"));

    QString url;
    quint16 port;
    bool enableProxy;
    enableProxy = CResources::self().getHttpProxy(url,port);
    m_networkAccessManager = new QNetworkAccessManager(this);

    if(enableProxy)
    {
        m_networkAccessManager->setProxy(QNetworkProxy(QNetworkProxy::DefaultProxy,url,port));
    }
    connect(m_networkAccessManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(slotRequestFinished(QNetworkReply*)));

    connect(&CRouteDB::self(), SIGNAL(sigChanged()), this, SLOT(slotDBChanged()));

    connect(listRoutes,SIGNAL(itemClicked(QListWidgetItem*) ),this,SLOT(slotItemClicked(QListWidgetItem*)));
    connect(listRoutes,SIGNAL(itemDoubleClicked(QListWidgetItem*) ),this,SLOT(slotItemDoubleClicked(QListWidgetItem*)));
    connect(listRoutes,SIGNAL(itemSelectionChanged()),this,SLOT(slotSelectionChanged()));
    connect(listRoutes,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(slotContextMenu(const QPoint&)));

    tabWidget->setTabIcon(eTabRoute, QIcon(":/icons/iconRoute16x16.png"));
    tabWidget->setTabIcon(eTabSetup, QIcon(":/icons/iconConfig16x16.png"));
    tabWidget->setTabIcon(eTabHelp, QIcon(":/icons/iconHelp16x16.png"));

    comboService->addItem("OpenRouteService (Europe)", eOpenRouteService);
    comboService->addItem("MapQuest (World)", eMapQuest);

    comboPreference->addItem(tr("Fastest"), "Fastest");
    comboPreference->addItem(tr("Shortest"), "Shortest");
    comboPreference->addItem(tr("Bicycle"), "Bicycle");
    comboPreference->addItem(tr("Mountain bike"), "BicycleMTB");
    comboPreference->addItem(tr("Bicycle racer"), "BicycleRacer");
    comboPreference->addItem(tr("Bicycle safest"), "BicycleSafety");
    comboPreference->addItem(tr("Bicycle route"), "BicycleRoute");
    comboPreference->addItem(tr("Pedestrian"), "Pedestrian");

    comboLanguage->addItem(tr("English"), "en");
    comboLanguage->addItem(tr("German"), "de");
    comboLanguage->addItem(tr("Bulgarian"), "bg");
    comboLanguage->addItem(tr("Czech"), "cz");
    comboLanguage->addItem(tr("Dutch"), "nl");
    comboLanguage->addItem(tr("Croatian"), "hr");
    comboLanguage->addItem(tr("Hungarian"), "hu");
    comboLanguage->addItem(tr("Dutch (belgium)"), "nl_BE");
    comboLanguage->addItem(tr("Spanish"), "es");
    comboLanguage->addItem(tr("Esperanto"), "eo");
    comboLanguage->addItem(tr("Finnish"), "fi");
    comboLanguage->addItem(tr("French"), "fr");
    comboLanguage->addItem(tr("Italian"), "it");
    comboLanguage->addItem(tr("Portuguese (brazil)"), "pt_BR");
    comboLanguage->addItem(tr("Romanian"), "ro");
    comboLanguage->addItem(tr("Russian"), "ru");
    comboLanguage->addItem(tr("Svenska"), "se");
    comboLanguage->addItem(tr("Danish"), "dk");
    comboLanguage->addItem(tr("Turkish"), "tr");
    comboLanguage->addItem(tr("Catalan"), "ca");
    comboLanguage->addItem(tr("Japanese"), "ja");
    comboLanguage->addItem(tr("Norwegian"), "no");
    comboLanguage->addItem(tr("Vietnamese"), "vi");
    comboLanguage->addItem(tr("Norwegian-bokmal"), "nb");
    comboLanguage->addItem(tr("de - Rhenish"), "de-rheinl");
    comboLanguage->addItem(tr("de - Op Platt"), "de-opplat");
    comboLanguage->addItem(tr("de - Berlin dialect"), "de-berlin");
    comboLanguage->addItem(tr("de - Swabian"), "de-swabia");
    comboLanguage->addItem(tr("de - Ruhrpott"), "de-ruhrpo");
    comboLanguage->addItem(tr("de - great Austrian dialect"), "de-at-ooe");
    comboLanguage->addItem(tr("de - Bavarian"), "de-bay");

    QString locale = QLocale::system().name().left(2);
    int langIdx = comboLanguage->findData(locale);

    QSettings cfg;
    cfg.beginGroup("routing");
    comboService->setCurrentIndex(cfg.value("service", 0).toInt());
    comboPreference->setCurrentIndex(cfg.value("preference", 0).toInt());
    checkAvoidHighways->setChecked(cfg.value("avoidHighways", false).toBool());
    checkAvoidTollways->setChecked(cfg.value("avoidTollways", false).toBool());
    comboLanguage->setCurrentIndex(cfg.value("language", langIdx).toInt());
    cfg.endGroup();

    slotSetupLink();
    connect(&CResources::self(), SIGNAL(sigProxyChanged()), this, SLOT(slotSetupLink()));

    timer = new QTimer(this);
    timer->setSingleShot(true);

    connect(toolSortAlpha, SIGNAL(clicked()), this, SLOT(slotDBChanged()));
    connect(toolSortTime, SIGNAL(clicked()), this, SLOT(slotDBChanged()));

    toolSortAlpha->setIcon(QPixmap(":/icons/iconDec16x16.png"));
    toolSortTime->setIcon(QPixmap(":/icons/iconTime16x16.png"));

    toolSortAlpha->setChecked(cfg.value("route/sortAlpha", true).toBool());
    toolSortTime->setChecked(cfg.value("route/sortTime", true).toBool());


    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
}


CRouteToolWidget::~CRouteToolWidget()
{
    QSettings cfg;
    cfg.setValue("route/sortAlpha", toolSortAlpha->isChecked());
    cfg.setValue("route/sortTime", toolSortTime->isChecked());

}


void CRouteToolWidget::slotDBChanged()
{
    if(originator) return;

    if(toolSortAlpha->isChecked())
    {
        sortmode = eSortByName;
    }
    else if(toolSortTime->isChecked())
    {
        sortmode = eSortByTime;
    }

    QFontMetrics fm(listRoutes->font());
    QPixmap icon(16,N_LINES*fm.height());
    icon.fill(Qt::white);

    listRoutes->clear();
    listRoutes->setIconSize(icon.size());

    QListWidgetItem * highlighted = 0;

    CRouteDB::keys_t key;
    QList<CRouteDB::keys_t> keys = CRouteDB::self().keys();

    foreach(key, keys)
    {
        CRoute * route = CRouteDB::self().getRouteByKey(key.key);

        QListWidgetItem * item = new QListWidgetItem(listRoutes);

        icon.fill(Qt::transparent);
        QPainter p;
        p.begin(&icon);
        p.drawPixmap(0,0,route->getIcon());
        p.end();

        item->setText(route->getInfo());
        item->setData(Qt::UserRole, route->getKey());
        item->setIcon(icon);

        if(route->isHighlighted())
        {
            highlighted = item;
        }

        ++route;
    }

    if(highlighted)
    {
        listRoutes->setCurrentItem(highlighted);
    }
}


void CRouteToolWidget::slotItemClicked(QListWidgetItem * item)
{
    originator = true;
    CRouteDB::self().highlightRoute(item->data(Qt::UserRole).toString());
    originator = false;
}


void CRouteToolWidget::slotItemDoubleClicked(QListWidgetItem * item)
{
    QString key = item->data(Qt::UserRole).toString();

    QRectF r = CRouteDB::self().getBoundingRectF(key);
    if (!r.isNull ())
    {
        CMapDB::self().getMap().zoom(r.left() * DEG_TO_RAD, r.top() * DEG_TO_RAD, r.right() * DEG_TO_RAD, r.bottom() * DEG_TO_RAD);
    }
}


void CRouteToolWidget::keyPressEvent(QKeyEvent * e)
{
    if(e->key() == Qt::Key_Delete)
    {
        slotDelete();
        e->accept();
    }
    else
    {
        QWidget::keyPressEvent(e);
    }
}


void CRouteToolWidget::slotContextMenu(const QPoint& pos)
{
    QListWidgetItem * item = listRoutes->currentItem();
    if(item)
    {
        QPoint p = listRoutes->mapToGlobal(pos);

        QMenu contextMenu;
        contextMenu.addAction(QPixmap(":/icons/iconEdit16x16.png"),tr("Edit"),this,SLOT(slotEdit()));
        contextMenu.addAction(QPixmap(":/icons/iconWizzard16x16.png"),tr("Calc. route"),this,SLOT(slotCalcRoute()));
        contextMenu.addSeparator();
        contextMenu.addAction(QPixmap(":/icons/iconDistance16x16.png"),tr("Make Overlay"),this,SLOT(slotToOverlay()));
        contextMenu.addAction(QPixmap(":/icons/iconTrack16x16.png"),tr("Make Track"),this,SLOT(slotToTrack()));
        contextMenu.addSeparator();
        contextMenu.addAction(QPixmap(":/icons/iconZoomArea16x16.png"),tr("Zoom to fit"),this,SLOT(slotZoomToFit()));
        contextMenu.addAction(QPixmap(":/icons/iconClear16x16.png"),tr("Reset"),this,SLOT(slotResetRoute()));
        contextMenu.addAction(QPixmap(":/icons/iconClear16x16.png"),tr("Delete"),this,SLOT(slotDelete()),Qt::CTRL + Qt::Key_Delete);
        contextMenu.exec(p);
    }
}


void CRouteToolWidget::slotEdit()
{
    QListWidgetItem * item = listRoutes->currentItem();
    if(item == 0) return;

    QString key     = item->data(Qt::UserRole).toString();
    CRoute* route   = CRouteDB::self().getRouteByKey(key);
    if(route == 0) return;

    CDlgEditRoute dlg(*route, this);
    dlg.exec();
}


void CRouteToolWidget::slotDelete()
{
    QStringList keys;
    QListWidgetItem * item;
    const QList<QListWidgetItem*>& items = listRoutes->selectedItems();
    foreach(item,items)
    {
        keys << item->data(Qt::UserRole).toString();
    }
    originator = true;
    CRouteDB::self().delRoutes(keys);
    originator = false;

    slotDBChanged();
}


void CRouteToolWidget::slotSetupLink()
{
    QString url;
    quint16 port;
    bool enableProxy;

    enableProxy = CResources::self().getHttpProxy(url,port);

    if(enableProxy)
    {
         m_networkAccessManager->setProxy(QNetworkProxy(QNetworkProxy::DefaultProxy,url,port));
    }
}

void CRouteToolWidget::slotCalcRoute()
{

    QListWidgetItem * item;
    QList<QListWidgetItem *> items = listRoutes->selectedItems();

    QSettings cfg;
    cfg.beginGroup("routing");
    cfg.setValue("service", comboService->currentIndex());
    cfg.setValue("preference", comboPreference->currentIndex());
    cfg.setValue("avoidHighways", checkAvoidHighways->isChecked());
    cfg.setValue("avoidTollways", checkAvoidTollways->isChecked());
    cfg.setValue("language", comboLanguage->currentIndex());
    cfg.endGroup();


    originator = true;
    foreach(item, items)
    {
        QString key     = item->data(Qt::UserRole).toString();
        CRoute* route   = CRouteDB::self().getRouteByKey(key);
        if(route == 0) return;

        route->setCalcPending();
        route->reset();

        qint32 service = comboService->itemData(comboService->currentIndex()).toInt();

        if(service == eOpenRouteService)
        {
            startOpenRouteService(*route);
        }
    }

    originator = false;
    slotDBChanged();
}




const QString CRouteToolWidget::gml_ns = "http://www.opengis.net/gml";
const QString CRouteToolWidget::xls_ns = "http://www.opengis.net/xls";
const QString CRouteToolWidget::xsi_ns = "http://www.w3.org/2001/XMLSchema-instance";
const QString CRouteToolWidget::sch_ns = "http://www.ascc.net/xml/schematron";
const QString CRouteToolWidget::xlink_ns = "http://www.w3.org/1999/xlink";
const QString CRouteToolWidget::schemaLocation = "http://www.opengis.net/xls http://schemas.opengis.net/ols/1.1.0/RouteService.xsd";


void CRouteToolWidget::startOpenRouteService(CRoute& rte)
{
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
    root.setAttribute("xls:lang", comboLanguage->itemData(comboLanguage->currentIndex()).toString());

    QDomElement requestHeader = xml.createElement("xls:RequestHeader");
    root.appendChild(requestHeader);

    QDomElement Request = xml.createElement("xls:Request");
    root.appendChild(Request);

    Request.setAttribute("methodName", "RouteRequest");
    Request.setAttribute("requestID", rte.getKey());
    Request.setAttribute("version", "1.1");


    QDomElement DetermineRouteRequest = xml.createElement("xls:DetermineRouteRequest");
    Request.appendChild(DetermineRouteRequest);

    DetermineRouteRequest.setAttribute("distanceUnit", "KM");

    QDomElement RoutePlan = xml.createElement("xls:RoutePlan");
    DetermineRouteRequest.appendChild(RoutePlan);

    QDomElement RoutePreference = xml.createElement("xls:RoutePreference");
    RoutePlan.appendChild(RoutePreference);

    QDomText _RoutePreference_ = xml.createTextNode(comboPreference->itemData(comboPreference->currentIndex()).toString());
    RoutePreference.appendChild(_RoutePreference_);

    QDomElement WayPointList = xml.createElement("xls:WayPointList");
    RoutePlan.appendChild(WayPointList);

    addOpenLSWptList(xml, WayPointList, rte);

    QDomElement AvoidList = xml.createElement("xls:AvoidList");
    RoutePlan.appendChild(AvoidList);
    if(checkAvoidHighways->isChecked())
    {
        QDomElement AvoidFeature = xml.createElement("xls:AvoidFeature");
        AvoidList.appendChild(AvoidFeature);

        QDomText _AvoidFeature_ = xml.createTextNode("Highway");
        AvoidFeature.appendChild(_AvoidFeature_);
    }

    if(checkAvoidTollways->isChecked())
    {
        QDomElement AvoidFeature = xml.createElement("xls:AvoidFeature");
        AvoidList.appendChild(AvoidFeature);

        QDomText _AvoidFeature_ = xml.createTextNode("Tollway");
        AvoidFeature.appendChild(_AvoidFeature_);
    }



    QDomElement RouteInstructionsRequest = xml.createElement("xls:RouteInstructionsRequest");
    RouteInstructionsRequest.setAttribute("provideGeometry", "1");
    DetermineRouteRequest.appendChild(RouteInstructionsRequest);

    QByteArray array;
    QTextStream out(&array, QIODevice::WriteOnly);
    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << xml.toString() << endl;

    QUrl url("http://openls.geog.uni-heidelberg.de");
    url.setPath("/qlandkarte/route");

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("User-Agent", WHAT_STR);

    m_networkAccessManager->post(request, array);

    timer->start(15000);

}

void CRouteToolWidget::addOpenLSWptList(QDomDocument& xml, QDomElement& WayPointList, CRoute& rte)
{

    QVector<CRoute::pt_t> wpts = rte.getPriRtePoints();

    QDomElement StartPoint = xml.createElement("xls:StartPoint");
    WayPointList.appendChild(StartPoint);
    addOpenLSPos(xml, StartPoint, wpts.first());

    if(wpts.size() > 2)
    {
        const int size = wpts.size() - 1;
        for(int i = 1; i < size; i++)
        {

            QDomElement ViaPoint = xml.createElement("xls:ViaPoint");
            WayPointList.appendChild(ViaPoint);
            addOpenLSPos(xml, ViaPoint, wpts[i]);
        }
    }

    QDomElement EndPoint = xml.createElement("xls:EndPoint");
    WayPointList.appendChild(EndPoint);
    addOpenLSPos(xml, EndPoint, wpts.last());
}

void CRouteToolWidget::addOpenLSPos(QDomDocument& xml, QDomElement& Parent, CRoute::pt_t& pt)
{
    QString lon, lat;
    QDomElement Position = xml.createElement("xls:Position");
    Parent.appendChild(Position);

    QDomElement Point = xml.createElement("gml:Point");
    Point.setAttribute("srsName", "EPSG:4326");
    Position.appendChild(Point);

    QDomElement Pos = xml.createElement("gml:pos");
    Point.appendChild(Pos);

    lon.sprintf("%1.8f", pt.lon);
    lat.sprintf("%1.8f", pt.lat);

    QDomText _Pos_ = xml.createTextNode(QString("%1 %2").arg(lon).arg(lat));
    Pos.appendChild(_Pos_);
}


void CRouteToolWidget::slotRequestFinished(QNetworkReply* reply)
{
    if(reply->error() != QNetworkReply::NoError)
    {
        timer->stop();
        QMessageBox::warning(0,tr("Failed..."), tr("Bad response from server:\n%1").arg(reply->errorString()), QMessageBox::Abort);
        reply->deleteLater();
        return;
    }

    QByteArray res = reply->readAll();
    reply->deleteLater();

    if(res.isEmpty())
    {
        return;
    }

    timer->stop();

    QDomDocument xml;
    xml.setContent(res);

    qDebug() << xml.toString();

    QDomElement root     = xml.documentElement();
    QDomElement response = root.firstChildElement("xls:Response");

    if(response.isNull())
    {
        QMessageBox::warning(0,tr("Failed..."), tr("Bad response from server:\n%1").arg(xml.toString()), QMessageBox::Abort);
        return;
    }

    QString key = response.attribute("requestID","");
    CRouteDB::self().loadSecondaryRoute(key, xml);
}

void CRouteToolWidget::slotResetRoute()
{
    QListWidgetItem * item;
    QList<QListWidgetItem *> items = listRoutes->selectedItems();

    foreach(item, items)
    {
        QString key     = item->data(Qt::UserRole).toString();
        CRouteDB::self().reset(key);
    }
}

void CRouteToolWidget::slotSelectionChanged()
{
    if(originator)
    {
        return;
    }
    if(listRoutes->hasFocus() && listRoutes->selectedItems().isEmpty())
    {
        CRouteDB::self().highlightRoute("");
    }
}

void CRouteToolWidget::slotToOverlay()
{

    QListWidgetItem * item;
    const QList<QListWidgetItem*>& items = listRoutes->selectedItems();
    foreach(item,items)
    {
        CRoute * route = CRouteDB::self().getRouteByKey(item->data(Qt::UserRole).toString());

        QList<COverlayDistance::pt_t> pts;

        int idx = 0;
        CRoute::pt_t rtept;
        QVector<CRoute::pt_t>& rtepts = route->getSecRtePoints().isEmpty() ? route->getPriRtePoints() : route->getSecRtePoints();
        foreach(rtept, rtepts)
        {
            COverlayDistance::pt_t pt;
            pt.u = rtept.lon * DEG_TO_RAD;
            pt.v = rtept.lat * DEG_TO_RAD;
            pt.idx = idx++;

            pts << pt;
        }

        COverlayDB::self().addDistance(route->getName(), tr("created from route"), 0.0, pts);
    }

    CMegaMenu::self().switchByKeyWord("Overlay");
}

void CRouteToolWidget::slotToTrack()
{

    QListWidgetItem * item;
    const QList<QListWidgetItem*>& items = listRoutes->selectedItems();
    foreach(item,items)
    {
        CRoute * route = CRouteDB::self().getRouteByKey(item->data(Qt::UserRole).toString());

        QVector<CRoute::pt_t>& rtepts = route->getSecRtePoints().isEmpty() ? route->getPriRtePoints() : route->getSecRtePoints();


        double dist, d, delta = 10.0, a1 , a2;
        XY pt1, pt2, ptx;
        CTrack::pt_t pt;
        CDlgConvertToTrack::EleMode_e eleMode;

        CDlgConvertToTrack dlg(0);
        if(dlg.exec() == QDialog::Rejected)
        {
            return;
        }

        CTrack * track  = new CTrack(&CTrackDB::self());
        track->setName(route->getName());

        delta   = dlg.getDelta();
        eleMode = dlg.getEleMode();


        if(delta == -1)
        {

            for(int i = 0; i < rtepts.count(); ++i)
            {
                pt2 = rtepts[i];
                pt.lon = pt2.u;
                pt.lat = pt2.v;
                pt._lon = pt.lon;
                pt._lat = pt.lat;
                *track << pt;
            }
        }
        else
        {
            if((route->getDistance() / delta) > (MAX_TRACK_SIZE - rtepts.count()))
            {
                delta = route->getDistance() / (MAX_TRACK_SIZE - rtepts.count());
            }

            // 1st point
            pt1 = rtepts.first();
            pt.lon = pt1.u;
            pt.lat = pt1.v;
            pt._lon = pt.lon;
            pt._lat = pt.lat;
            *track << pt;


            pt1.u = pt1.u * DEG_TO_RAD;
            pt1.v = pt1.v * DEG_TO_RAD;

            // all other points
            for(int i = 1; i < rtepts.count(); ++i)
            {
                pt2 = rtepts[i];

                pt2.u = pt2.u * DEG_TO_RAD;
                pt2.v = pt2.v * DEG_TO_RAD;


                // all points from pt1 -> pt2, with 10m steps
                dist = ::distance(pt1, pt2, a1, a2);
                a1 *= DEG_TO_RAD;

                d = delta;
                while(d < dist)
                {
                    ptx = GPS_Math_Wpt_Projection(pt1, d, a1);
                    pt.lon = ptx.u * RAD_TO_DEG;
                    pt.lat = ptx.v * RAD_TO_DEG;
                    pt._lon = pt.lon;
                    pt._lat = pt.lat;

                    *track << pt;

                    d += delta;
                }

                // and finally the next point
                pt.lon = pt2.u * RAD_TO_DEG;
                pt.lat = pt2.v * RAD_TO_DEG;
                pt._lon = pt.lon;
                pt._lat = pt.lat;

                *track << pt;

                pt1 = pt2;
            }
        }

        if(eleMode == CDlgConvertToTrack::eLocal)
        {
            track->replaceElevationByLocal();
        }
        else if(eleMode == CDlgConvertToTrack::eRemote)
        {
            track->replaceElevationByRemote();
        }

        CTrackDB::self().addTrack(track, false);
    }
    CMegaMenu::self().switchByKeyWord("Tracks");

}

void CRouteToolWidget::slotZoomToFit()
{
    QRectF r;

    const QList<QListWidgetItem*>& items = listRoutes->selectedItems();
    QList<QListWidgetItem*>::const_iterator item = items.begin();

    r = CRouteDB::self().getBoundingRectF((*item)->data(Qt::UserRole).toString());

    while(item != items.end())
    {
        r |= CRouteDB::self().getBoundingRectF((*item)->data(Qt::UserRole).toString());
        ++item;
    }

    if (!r.isNull ())
    {
        CMapDB::self().getMap().zoom(r.left() * DEG_TO_RAD, r.top() * DEG_TO_RAD, r.right() * DEG_TO_RAD, r.bottom() * DEG_TO_RAD);
    }
}

void CRouteToolWidget::slotTimeout()
{
    QMessageBox::warning(0,tr("Failed..."), tr("Route request timed out. Please try again later."), QMessageBox::Abort);
}
