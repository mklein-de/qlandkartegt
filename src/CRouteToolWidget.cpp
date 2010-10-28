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

#include <QtGui>
#include <QtXml>
#include <QHttp>

CRouteToolWidget::CRouteToolWidget(QTabWidget * parent)
: QWidget(parent)
, originator(false)
, http(0)
{
    setupUi(this);
    setObjectName("Routes");
    parent->addTab(this,QIcon(":/icons/iconRoute16x16.png"),"");
    parent->setTabToolTip(parent->indexOf(this), tr("Routes"));

    connect(&CRouteDB::self(), SIGNAL(sigChanged()), this, SLOT(slotDBChanged()));

    connect(listRoutes,SIGNAL(itemClicked(QListWidgetItem*) ),this,SLOT(slotItemClicked(QListWidgetItem*)));
    connect(listRoutes,SIGNAL(itemDoubleClicked(QListWidgetItem*) ),this,SLOT(slotItemDoubleClicked(QListWidgetItem*)));
    connect(listRoutes,SIGNAL(itemSelectionChanged()),this,SLOT(slotSelectionChanged()));
    connect(listRoutes,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(slotContextMenu(const QPoint&)));

    tabWidget->setTabIcon(eTabRoute, QIcon(":/icons/iconRoute16x16.png"));
    tabWidget->setTabIcon(eTabSetup, QIcon(":/icons/iconConfig16x16.png"));
    tabWidget->setTabIcon(eTabHelp, QIcon(":/icons/iconHelp16x16.png"));

    comboService->addItem("OpenRouteService", eOpenRouteService);

    comboPreference->addItem(tr("fastest"), "Fastest");
    comboPreference->addItem(tr("shortest"), "Shortest");
    comboPreference->addItem(tr("pedestrian"), "Pedestrian");

    QSettings cfg;
    cfg.beginGroup("routing");
    comboService->setCurrentIndex(cfg.value("service", 0).toInt());
    comboPreference->setCurrentIndex(cfg.value("preference", 0).toInt());
    checkAvoidHighways->setChecked(cfg.value("avoidHighways", false).toBool());
    checkAvoidTollways->setChecked(cfg.value("avoidTollways", false).toBool());
    cfg.endGroup();

    slotSetupLink();
    connect(&CResources::self(), SIGNAL(sigProxyChanged()), this, SLOT(slotSetupLink()));

}


CRouteToolWidget::~CRouteToolWidget()
{

}


void CRouteToolWidget::slotDBChanged()
{
    if(originator) return;

    listRoutes->clear();

    QListWidgetItem * highlighted = 0;

    const QMap<QString,CRoute*>& routes         = CRouteDB::self().getRoutes();
    QMap<QString,CRoute*>::const_iterator route = routes.begin();
    while(route != routes.end())
    {
        QListWidgetItem * item = new QListWidgetItem(listRoutes);

        item->setText((*route)->getInfo());
        item->setData(Qt::UserRole, (*route)->getKey());
        item->setIcon((*route)->getIcon());

        if((*route)->isHighlighted())
        {
            highlighted = item;
        }

        ++route;
    }

    listRoutes->sortItems();

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
        delete listRoutes->takeItem(listRoutes->row(item));
    }
    originator = true;
    CRouteDB::self().delRoutes(keys);
    originator = false;
}


void CRouteToolWidget::slotSetupLink()
{
    QString url;
    quint16 port;
    bool enableProxy;

    enableProxy = CResources::self().getHttpProxy(url,port);

    if(http) delete http;
    http = new QHttp(this);
    if(enableProxy)
    {
        http->setProxy(url,port);
    }

    connect(http,SIGNAL(requestStarted(int)),this,SLOT(slotRequestStarted(int)));
    connect(http,SIGNAL(requestFinished(int,bool)),this,SLOT(slotRequestFinished(int,bool)));
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
    cfg.endGroup();


    foreach(item, items)
    {

        QString key     = item->data(Qt::UserRole).toString();
        CRoute* route   = CRouteDB::self().getRouteByKey(key);
        if(route == 0) return;

        qint32 service = comboService->itemData(comboService->currentIndex()).toInt();

        if(service == eOpenRouteService)
        {
            startOpenRouteService(*route);
        }
    }
}




const QString CRouteToolWidget::gml_ns = "http://www.opengis.net/gml";
const QString CRouteToolWidget::xls_ns = "http://www.opengis.net/xls";
const QString CRouteToolWidget::xsi_ns = "http://www.w3.org/2001/XMLSchema-instance";
const QString CRouteToolWidget::sch_ns = "http://www.ascc.net/xml/schematron";
const QString CRouteToolWidget::xlink_ns = "http://www.w3.org/1999/xlink";
const QString CRouteToolWidget::schemaLocation = "http://www.opengis.net/xls http://schemas.opengis.net/ols/1.1.0/RouteService.xsd";


void CRouteToolWidget::startOpenRouteService(CRoute& rte)
{
    if(http == 0) return;

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
    root.setAttribute("xls:lang",QLocale::system().name().left(2));

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

    http->setHost("openls.geog.uni-heidelberg.de");

    QUrl url;
    url.setPath("/qlandkarte/route");

    http->post(url.toEncoded(), array);

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

void CRouteToolWidget::slotRequestStarted(int )
{

}


void CRouteToolWidget::slotRequestFinished(int , bool error)
{
    if(error)
    {
        QMessageBox::warning(0,tr("Failed..."), tr("Bad response from server:\n%1").arg(http->errorString()), QMessageBox::Abort);
        return;
    }

    QByteArray res = http->readAll();
    if(res.isEmpty())
    {
        return;
    }

    QDomDocument xml;
    xml.setContent(res);

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

