/**********************************************************************************************
    Copyright (C) 2008 Oliver Eichler oliver.eichler@gmx.de

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
#include "CMapTDB.h"
#include "CMapDB.h"
#include "CMapDEM.h"
#include "Garmin.h"
#include "CGarminTile.h"
#ifdef SQL_SEARCH_GARMIN
#include "CGarminIndex.h"
#endif //SQL_SEARCH_GARMIN
#include "GeoMath.h"
#include "CResources.h"
#include "CMainWindow.h"
#include "CCanvas.h"
#include "IUnit.h"
#include "Platform.h"
#include "CMapSelectionGarmin.h"
#include "CDlgMapTDBConfig.h"
#include "CGarminTyp.h"
#include "CGarminTypNT.h"

#include <QtGui>
#include <algorithm>
#include <QSqlDatabase>

#define MAX_IDX_ZOOM 35
#define MIN_IDX_ZOOM 0
#undef DEBUG_SHOW_SECTION_BORDERS
#define DEBUG_SHOW_MAPLEVELS

#define TEXTWIDTH   300

CMapTDB::scale_t CMapTDB::scales[] =
{
    {                            //0
        QString("7000 km"), 70000.0, 8
    }
    ,                            //1
    {
        QString("5000 km"), 50000.0, 8
    }
    ,                            //2
    {
        QString("3000 km"), 30000.0, 9
    }
    ,                            //3
    {
        QString("2000 km"), 20000.0, 9
    }
    ,                            //4
    {
        QString("1500 km"), 15000.0, 10
    }
    ,                            //5
    {
        QString("1000 km"), 10000.0, 10
    }
    ,                            //6
    {
        QString("700 km"), 7000.0, 11
    }
    ,                            //7
    {
        QString("500 km"), 5000.0, 11
    }
    ,                            //8
    {
        QString("300 km"), 3000.0, 13
    }
    ,                            //9
    {
        QString("200 km"), 2000.0, 13
    }
    ,                            //10
    {
        QString("150 km"), 1500.0, 13
    }
    ,                            //11
    {
        QString("100 km"), 1000.0, 14
    }
    ,                            //12
    {
        QString("70 km"), 700.0, 15
    }
    ,                            //13
    {
        QString("50 km"), 500.0, 16
    }
    ,                            //14
    {
        QString("30 km"), 300.0, 16
    }
    ,                            //15
    {
        QString("20 km"), 200.0, 17
    }
    ,                            //16
    {
        QString("15 km"), 150.0, 17
    }
    ,                            //17
    {
        QString("10 km"), 100.0, 18
    }
    ,                            //18
    {
        QString("7 km"), 70.0, 18
    }
    ,                            //19
    {
        QString("5 km"), 50.0, 19
    }
    ,                            //20
    {
        QString("3 km"), 30.0, 19
    }
    ,                            //21
    {
        QString("2 km"), 20.0, 20
    }
    ,                            //22
    {
        QString("1.5 km"), 15.0, 22
    }
    ,                            //23
    {
        QString("1 km"), 10.0, 24
    }
    ,                            //24
    {
        QString("700 m"), 7.0, 24
    }
    ,                            //25
    {
        QString("500 m"), 5.0, 24
    }
    ,                            //26
    {
        QString("300 m"), 3.0, 24
    }
    ,                            //27
    {
        QString("200 m"), 2.0, 24
    }
    ,                            //28
    {
        QString("150 m"), 1.5, 24
    }
    ,                            //29
    {
        QString("100 m"), 1.0, 24
    }
    ,                            //30
    {
        QString("70 m"), 0.7, 24
    }
    ,                            //31
    {
        QString("50 m"), 0.5, 24
    }
    ,                            //32
    {
        QString("30 m"), 0.3, 24
    }
    ,                            //33
    {
        QString("20 m"), 0.2, 24
    }
    ,                            //34
    {
        QString("15 m"), 0.15, 24
    }
    ,                            //35
    {
        QString("10 m"), 0.10, 24
    }
};

CMapTDB::CMapTDB(const QString& key, const QString& filename, CCanvas * parent)
: IMap(eGarmin, key, parent)
, tainted(false)
, filename(filename)
, north(-90.0 * DEG_TO_RAD)
, east(-180.0 * DEG_TO_RAD)
, south(90.0 * DEG_TO_RAD)
, west(180.0 * DEG_TO_RAD)
, baseimg(0)
, isTransparent(false)
, zoomFactor(0)
, fm(CResources::self().getMapFont())
, detailsFineTune(0)
, lon_factor(+1.0)
, lat_factor(-1.0)
, useTyp(true)
, textAboveLine(true)
, poiLabels(true)
, checkPoiLabels(0)
, nightView(false)
, checkNightView(0)
, comboDetails(0)
, comboLanguages(0)
, selectedLanguage(-1)
{
    readTDB(filename);
    //     QString str = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs +towgs84=0,0,0";
    QString str = QString("+proj=merc +lat_ts=%1 +ellps=WGS84").arg(int((south + (north - south) / 2) * RAD_TO_DEG));
    pjsrc       = pj_init_plus(str.toLatin1());

    char * ptr;
    ptr = pj_get_def(pjsrc,0);
    qDebug() << "pjsrc:\t" << ptr;
    //     free(ptr);
    ptr = pj_get_def(pjtar,0);
    qDebug() << "pjtar:\t" << ptr;
    //     free(ptr);

    processPrimaryMapData();

    QSettings cfg;
    cfg.beginGroup("garmin/maps");
    cfg.beginGroup(name);
    QString pos     = cfg.value("topleft","").toString();
    zoomidx         = cfg.value("zoomidx",11).toInt();
    detailsFineTune = cfg.value("details",0).toInt();
    textAboveLine   = cfg.value("textAboveLine",textAboveLine).toBool();
    useTyp          = cfg.value("useTyp",useTyp).toBool();
    poiLabels       = cfg.value("poiLabels",poiLabels).toBool();
    nightView       = cfg.value("nightView",nightView).toBool();
    cfg.endGroup();
    cfg.endGroup();

    setup();

    if(pos.isEmpty())
    {
        topLeft.u = west;
        topLeft.v = north;
    }
    else
    {
        float u = 0;
        float v = 0;
        GPS_Math_Str_To_Deg(pos, u, v);
        topLeft.u = u * DEG_TO_RAD;
        topLeft.v = v * DEG_TO_RAD;
    }
    zoom(zoomidx);
    resize(parent->size());

    info = new QTextDocument(this);
    info->setTextWidth(TEXTWIDTH);
    info->setHtml(infotext);

    parent->installEventFilter(this);

#ifdef SQL_SEARCH_GARMIN
    index = new CGarminIndex(this);
    index->setDBName(name);
#endif //SQL_SEARCH_GARMIN

    checkPoiLabels = new QCheckBox(theMainWindow->getCanvas());
    checkPoiLabels->setText(tr("POI labels"));
    theMainWindow->statusBar()->insertPermanentWidget(0,checkPoiLabels);
    checkPoiLabels->setChecked(poiLabels);
    connect(checkPoiLabels, SIGNAL(toggled(bool)), this, SLOT(slotPoiLabels(bool)));

    checkNightView = new QCheckBox(theMainWindow->getCanvas());
    checkNightView->setText(tr("Night"));
    theMainWindow->statusBar()->insertPermanentWidget(0,checkNightView);
    checkNightView->setChecked(nightView);
    connect(checkNightView, SIGNAL(toggled(bool)), this, SLOT(slotNightView(bool)));

    comboDetails = new QComboBox(theMainWindow->getCanvas());
    comboDetails->addItem(tr("Detail  5"),  5);
    comboDetails->addItem(tr("Detail  4"),  4);
    comboDetails->addItem(tr("Detail  3"),  3);
    comboDetails->addItem(tr("Detail  2"),  2);
    comboDetails->addItem(tr("Detail  1"),  1);
    comboDetails->addItem(tr("Detail  0"),  0);
    comboDetails->addItem(tr("Detail -1"), -1);
    comboDetails->addItem(tr("Detail -2"), -2);
    comboDetails->addItem(tr("Detail -3"), -3);
    comboDetails->addItem(tr("Detail -4"), -4);
    comboDetails->addItem(tr("Detail -5"), -5);
    comboDetails->setCurrentIndex(comboDetails->findData(detailsFineTune));
    connect(comboDetails, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDetailChanged(int)));
    theMainWindow->statusBar()->insertPermanentWidget(0,comboDetails);

    qDebug() << "CMapTDB::CMapTDB()";

}


CMapTDB::CMapTDB(const QString& key, const QString& filename)
: IMap(eGarmin, key, 0)
, filename(filename)
, north(-90.0)
, east(-180.0)
, south(90.0)
, west(180.0)
, baseimg(0)
, isTransparent(false)
, zoomFactor(0)
, fm(CResources::self().getMapFont())
, detailsFineTune(0)
, fid(0x0001)
, pid(0x0320)
, lon_factor(+1.0)
, lat_factor(-1.0)
, useTyp(true)
, textAboveLine(true)
, poiLabels(true)
, checkPoiLabels(0)
, nightView(false)
, checkNightView(0)
, comboDetails(0)
, comboLanguages(0)
, selectedLanguage(-1)
{
    char * ptr = CMapDB::self().getMap().getProjection();

    if(QString(ptr).contains("longlat"))
    {
        lon_factor =   PI / pow(2.0, 24);
        lat_factor = - PI / pow(2.0, 24);
        qDebug() << "set correction factor to" << lon_factor << lat_factor;
    }

    pjsrc = pj_init_plus(ptr);

    qDebug() << "TDB:" << ptr;
    //     if(ptr) free(ptr);

    readTDB(filename);
    processPrimaryMapData();

    QSettings cfg;
    cfg.beginGroup("garmin/maps");
    cfg.beginGroup(name);
    detailsFineTune = cfg.value("details",0).toInt();
    textAboveLine   = cfg.value("textAboveLine",textAboveLine).toBool();
    useTyp          = cfg.value("useTyp",useTyp).toBool();
    poiLabels       = cfg.value("poiLabels",poiLabels).toBool();
    cfg.endGroup();
    cfg.endGroup();

    setup();
    info          = 0;
    isTransparent = true;

#ifdef SQL_SEARCH_GARMIN
    index = new CGarminIndex(this);
    index->setDBName(name);
#endif //SQL_SEARCH_GARMIN

    qDebug() << "CMapTDB::CMapTDB()";
}


CMapTDB::~CMapTDB()
{
    QString pos;
    QSettings cfg;
    cfg.beginGroup("garmin/maps");
    cfg.beginGroup(name);
    GPS_Math_Deg_To_Str(topLeft.u * RAD_TO_DEG, topLeft.v * RAD_TO_DEG, pos);
    pos = pos.replace("\260","");
    cfg.setValue("topleft",pos);
    cfg.setValue("zoomidx",zoomidx);
    cfg.setValue("details", detailsFineTune);
    cfg.setValue("textAboveLine",textAboveLine);
    cfg.setValue("useTyp",useTyp);
    cfg.setValue("poiLabels",poiLabels);
    cfg.setValue("nightView",nightView);
    cfg.setValue("selectedLanguage",selectedLanguage);

    cfg.endGroup();
    cfg.endGroup();

    midU = rect.center().x();
    midV = rect.center().y();
    convertPt2Rad(midU, midV);

    if(pjsrc) pj_free(pjsrc);

    if(checkPoiLabels)
    {
        delete checkPoiLabels;
    }

    if(checkNightView)
    {
        delete checkNightView;
    }

    if(comboDetails)
    {
        delete comboDetails;
    }

    if(comboLanguages)
    {
        delete comboLanguages;
    }

    qDebug() << "CMapTDB::~CMapTDB()";
}


void CMapTDB::slotPoiLabels(bool checked)
{
    poiLabels   = checked;
    needsRedraw = true;
    emit sigChanged();
}


void CMapTDB::slotNightView(bool checked)
{
    nightView   = checked;
    needsRedraw = true;
    emit sigChanged();
}


void CMapTDB::slotDetailChanged(int idx)
{

    detailsFineTune = comboDetails->itemData(idx).toInt();
    needsRedraw = true;
    emit sigChanged();
}


void CMapTDB::slotLanguageChanged(int idx)
{
    selectedLanguage = comboLanguages->itemData(idx).toInt();
    needsRedraw = true;
    emit sigChanged();
}


void CMapTDB::setup()
{
    languages.clear();
    languages[0x00] = tr("Unspecified");
    languages[0x01] = tr("French");
    languages[0x02] = tr("German");
    languages[0x03] = tr("Dutch");
    languages[0x04] = tr("English");
    languages[0x05] = tr("Italian");
    languages[0x06] = tr("Finnish");
    languages[0x07] = tr("Swedish");
    languages[0x08] = tr("Spanish");
    languages[0x09] = tr("Basque");
    languages[0x0a] = tr("Catalan");
    languages[0x0b] = tr("Galician");
    languages[0x0c] = tr("Welsh");
    languages[0x0d] = tr("Gaelic");
    languages[0x0e] = tr("Danish");
    languages[0x0f] = tr("Norwegian");
    languages[0x10] = tr("Portuguese");
    languages[0x11] = tr("Slovak");
    languages[0x12] = tr("Czech");
    languages[0x13] = tr("Croatian");
    languages[0x14] = tr("Hungarian");
    languages[0x15] = tr("Polish");
    languages[0x16] = tr("Turkish");
    languages[0x17] = tr("Greek");
    languages[0x18] = tr("Slovenian");
    languages[0x19] = tr("Russian");
    languages[0x1a] = tr("Estonian");
    languages[0x1b] = tr("Latvian");
    languages[0x1c] = tr("Romanian");
    languages[0x1d] = tr("Albanian");
    languages[0x1e] = tr("Bosnian");
    languages[0x1f] = tr("Lithuanian");
    languages[0x20] = tr("Serbian");
    languages[0x21] = tr("Macedonian");
    languages[0x22] = tr("Bulgarian");

    polylineProperties.clear();
    polylineProperties[0x01] = IGarminTyp::polyline_property(0x01, Qt::blue,   6, Qt::SolidLine );
    polylineProperties[0x01].penBorderDay = QPen(Qt::black, 8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    polylineProperties[0x01].penBorderNight = QPen(Qt::lightGray, 8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    polylineProperties[0x01].hasBorder = true;
    polylineProperties[0x02] = IGarminTyp::polyline_property(0x02, "#cc9900",   4, Qt::SolidLine );
    polylineProperties[0x02].penBorderDay = QPen(Qt::black, 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    polylineProperties[0x02].penBorderNight = QPen(Qt::lightGray, 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    polylineProperties[0x02].hasBorder = true;
    polylineProperties[0x03] = IGarminTyp::polyline_property(0x03, Qt::yellow,   3, Qt::SolidLine );
    polylineProperties[0x03].penBorderDay = QPen(Qt::black, 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    polylineProperties[0x03].penBorderNight = QPen(Qt::lightGray, 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    polylineProperties[0x03].hasBorder = true;
    polylineProperties[0x04] = IGarminTyp::polyline_property(0x04, "#ffff00",   3, Qt::SolidLine );
    polylineProperties[0x04].penBorderDay = QPen(Qt::black, 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    polylineProperties[0x04].penBorderNight = QPen(Qt::lightGray, 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    polylineProperties[0x04].hasBorder = true;
    polylineProperties[0x05] = IGarminTyp::polyline_property(0x05, "#dc7c5a",   2, Qt::SolidLine );
    polylineProperties[0x06] = IGarminTyp::polyline_property(0x06, Qt::gray,   2, Qt::SolidLine );
    polylineProperties[0x06].penBorderDay = QPen(Qt::black, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    polylineProperties[0x06].penBorderNight = QPen(QColor("#f0f0f0"), 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    polylineProperties[0x06].hasBorder = true;
    polylineProperties[0x07] = IGarminTyp::polyline_property(0x07, "#c46442",   1, Qt::SolidLine );
    polylineProperties[0x08] = IGarminTyp::polyline_property(0x08, "#e88866",   2, Qt::SolidLine );
    polylineProperties[0x09] = IGarminTyp::polyline_property(0x09, "#e88866",   2, Qt::SolidLine );
    polylineProperties[0x0A] = IGarminTyp::polyline_property(0x0A, "#808080",   2, Qt::SolidLine );
    polylineProperties[0x0B] = IGarminTyp::polyline_property(0x0B, "#c46442",   2, Qt::SolidLine );
    polylineProperties[0x0C] = IGarminTyp::polyline_property(0x0C, "#FFFFFF",   2, Qt::SolidLine );
    polylineProperties[0x14] = IGarminTyp::polyline_property(0x14, Qt::white,   2, Qt::DotLine   );
    polylineProperties[0x14].penBorderDay = QPen(Qt::black, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    polylineProperties[0x14].penBorderNight = QPen(Qt::lightGray, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    polylineProperties[0x14].hasBorder = true;
    polylineProperties[0x15] = IGarminTyp::polyline_property(0x15, "#000080",   2, Qt::SolidLine );
    polylineProperties[0x16] = IGarminTyp::polyline_property(0x16, "#E0E0E0",   1, Qt::SolidLine );
    polylineProperties[0x18] = IGarminTyp::polyline_property(0x18, "#0000ff",   2, Qt::SolidLine );
    polylineProperties[0x19] = IGarminTyp::polyline_property(0x19, "#00ff00",   2, Qt::SolidLine );
    polylineProperties[0x1A] = IGarminTyp::polyline_property(0x1A, "#000000",   2, Qt::SolidLine );
    polylineProperties[0x1B] = IGarminTyp::polyline_property(0x1B, "#000000",   2, Qt::SolidLine );
    polylineProperties[0x1C] = IGarminTyp::polyline_property(0x1C, "#00c864",   2, Qt::DotLine   );
    polylineProperties[0x1D] = IGarminTyp::polyline_property(0x1D, "#00c864",   2, Qt::DotLine   );
    polylineProperties[0x1E] = IGarminTyp::polyline_property(0x1E, "#00c864",   2, Qt::DotLine   );
    polylineProperties[0x1F] = IGarminTyp::polyline_property(0x1F, "#0000ff",   2, Qt::SolidLine );
    polylineProperties[0x20] = IGarminTyp::polyline_property(0x20, "#b67824",   1, Qt::SolidLine );
    polylineProperties[0x21] = IGarminTyp::polyline_property(0x21, "#b67824",   2, Qt::SolidLine );
    polylineProperties[0x22] = IGarminTyp::polyline_property(0x22, "#b67824",   3, Qt::SolidLine );
    polylineProperties[0x23] = IGarminTyp::polyline_property(0x23, "#b67824",   1, Qt::SolidLine );
    polylineProperties[0x24] = IGarminTyp::polyline_property(0x24, "#b67824",   2, Qt::SolidLine );
    polylineProperties[0x25] = IGarminTyp::polyline_property(0x25, "#b67824",   3, Qt::SolidLine );
    polylineProperties[0x26] = IGarminTyp::polyline_property(0x26, "#0000ff",   2, Qt::DotLine   );
    polylineProperties[0x27] = IGarminTyp::polyline_property(0x27, "#c46442",   4, Qt::SolidLine );
    polylineProperties[0x28] = IGarminTyp::polyline_property(0x28, "#aa0000",   2, Qt::SolidLine );
    polylineProperties[0x29] = IGarminTyp::polyline_property(0x29, "#ff0000",   2, Qt::SolidLine );
    polylineProperties[0x2A] = IGarminTyp::polyline_property(0x2A, "#000000",   2, Qt::SolidLine );
    polylineProperties[0x2B] = IGarminTyp::polyline_property(0x2B, "#000000",   2, Qt::SolidLine );

    polylineProperties[0x01].strings[0x00] = tr("Major highway");
    polylineProperties[0x02].strings[0x00] = tr("Principal highway");
    polylineProperties[0x03].strings[0x00] = tr("Other highway");
    polylineProperties[0x04].strings[0x00] = tr("Arterial road");
    polylineProperties[0x05].strings[0x00] = tr("Collector road");
    polylineProperties[0x06].strings[0x00] = tr("Residential street");
    polylineProperties[0x07].strings[0x00] = tr("Alley/Private road");
    polylineProperties[0x08].strings[0x00] = tr("Highway ramp, low speed");
    polylineProperties[0x09].strings[0x00] = tr("Highway ramp, high speed");
    polylineProperties[0x0a].strings[0x00] = tr("Unpaved road");
    polylineProperties[0x0b].strings[0x00] = tr("Major highway connector");
    polylineProperties[0x0c].strings[0x00] = tr("Roundabout");
    polylineProperties[0x14].strings[0x00] = tr("Railroad");
    polylineProperties[0x15].strings[0x00] = tr("Shoreline");
    polylineProperties[0x16].strings[0x00] = tr("Trail");
    polylineProperties[0x18].strings[0x00] = tr("Stream");
    polylineProperties[0x19].strings[0x00] = tr("Time zone");
    polylineProperties[0x1a].strings[0x00] = tr("Ferry");
    polylineProperties[0x1b].strings[0x00] = tr("Ferry");
    polylineProperties[0x1c].strings[0x00] = tr("State/province border");
    polylineProperties[0x1d].strings[0x00] = tr("County/parish border");
    polylineProperties[0x1e].strings[0x00] = tr("International border");
    polylineProperties[0x1f].strings[0x00] = tr("River");
    polylineProperties[0x20].strings[0x00] = tr("Minor land contour");
    polylineProperties[0x21].strings[0x00] = tr("Intermediate land contour");
    polylineProperties[0x22].strings[0x00] = tr("Major land contour");
    polylineProperties[0x23].strings[0x00] = tr("Minor deph contour");
    polylineProperties[0x24].strings[0x00] = tr("Intermediate depth contour");
    polylineProperties[0x25].strings[0x00] = tr("Major depth contour");
    polylineProperties[0x26].strings[0x00] = tr("Intermittent stream");
    polylineProperties[0x27].strings[0x00] = tr("Airport runway");
    polylineProperties[0x28].strings[0x00] = tr("Pipeline");
    polylineProperties[0x29].strings[0x00] = tr("Powerline");
    polylineProperties[0x2a].strings[0x00] = tr("Marine boundary");
    polylineProperties[0x2b].strings[0x00] = tr("Hazard boundary");

    polygonProperties.clear();
    polygonProperties[0x01] = IGarminTyp::polygon_property(0x01, Qt::NoPen,     "#d2c0c0", Qt::SolidPattern);
    polygonProperties[0x02] = IGarminTyp::polygon_property(0x02, Qt::NoPen,     "#fbeab7", Qt::SolidPattern);
    polygonProperties[0x03] = IGarminTyp::polygon_property(0x03, Qt::NoPen,     "#a4b094", Qt::SolidPattern);
    polygonProperties[0x04] = IGarminTyp::polygon_property(0x04, Qt::NoPen,     "#808080", Qt::SolidPattern);
    polygonProperties[0x05] = IGarminTyp::polygon_property(0x05, Qt::NoPen,     "#f0f0f0", Qt::SolidPattern);
    polygonProperties[0x06] = IGarminTyp::polygon_property(0x06, Qt::NoPen,     "#cacaca", Qt::SolidPattern);
    polygonProperties[0x07] = IGarminTyp::polygon_property(0x07, Qt::NoPen,     "#feebcf", Qt::SolidPattern);
    polygonProperties[0x08] = IGarminTyp::polygon_property(0x08, Qt::NoPen,     "#fde8d5", Qt::SolidPattern);
    polygonProperties[0x09] = IGarminTyp::polygon_property(0x09, Qt::NoPen,     "#fee8b8", Qt::SolidPattern);
    polygonProperties[0x0a] = IGarminTyp::polygon_property(0x0a, Qt::NoPen,     "#fdeac6", Qt::SolidPattern);
    polygonProperties[0x0b] = IGarminTyp::polygon_property(0x0b, Qt::NoPen,     "#fddfbd", Qt::SolidPattern);
    polygonProperties[0x0c] = IGarminTyp::polygon_property(0x0c, Qt::NoPen,     "#ebeada", Qt::SolidPattern);
    polygonProperties[0x0d] = IGarminTyp::polygon_property(0x0d, Qt::NoPen,     "#f8e3be", Qt::SolidPattern);
    polygonProperties[0x0e] = IGarminTyp::polygon_property(0x0e, Qt::NoPen,     "#e0e0e0", Qt::SolidPattern);
    polygonProperties[0x13] = IGarminTyp::polygon_property(0x13, Qt::NoPen,     "#cc9900", Qt::SolidPattern);
    polygonProperties[0x14] = IGarminTyp::polygon_property(0x14, Qt::NoPen,     "#b7e999", Qt::SolidPattern);
    polygonProperties[0x15] = IGarminTyp::polygon_property(0x15, Qt::NoPen,     "#b7e999", Qt::SolidPattern);
    polygonProperties[0x16] = IGarminTyp::polygon_property(0x16, Qt::NoPen,     "#b7e999", Qt::SolidPattern);
    polygonProperties[0x17] = IGarminTyp::polygon_property(0x17, Qt::NoPen,     "#90be00", Qt::SolidPattern);
    polygonProperties[0x18] = IGarminTyp::polygon_property(0x18, Qt::NoPen,     "#00ff00", Qt::SolidPattern);
    polygonProperties[0x19] = IGarminTyp::polygon_property(0x19, Qt::NoPen,     "#f8e3be", Qt::SolidPattern);
    polygonProperties[0x1a] = IGarminTyp::polygon_property(0x1a, Qt::NoPen,     "#d3f5a5", Qt::SolidPattern);
    polygonProperties[0x1e] = IGarminTyp::polygon_property(0x1e, Qt::NoPen,     "#b7e999", Qt::SolidPattern);
    polygonProperties[0x1f] = IGarminTyp::polygon_property(0x1f, Qt::NoPen,     "#b7e999", Qt::SolidPattern);
    polygonProperties[0x20] = IGarminTyp::polygon_property(0x20, Qt::NoPen,     "#b7e999", Qt::SolidPattern);
    polygonProperties[0x28] = IGarminTyp::polygon_property(0x28, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x29] = IGarminTyp::polygon_property(0x29, Qt::NoPen,     "#0000ff", Qt::SolidPattern);
    polygonProperties[0x32] = IGarminTyp::polygon_property(0x32, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x3b] = IGarminTyp::polygon_property(0x3b, Qt::NoPen,     "#0000ff", Qt::SolidPattern);
    polygonProperties[0x3c] = IGarminTyp::polygon_property(0x3c, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x3d] = IGarminTyp::polygon_property(0x3d, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x3e] = IGarminTyp::polygon_property(0x3e, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x3f] = IGarminTyp::polygon_property(0x3f, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x40] = IGarminTyp::polygon_property(0x40, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x41] = IGarminTyp::polygon_property(0x41, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x42] = IGarminTyp::polygon_property(0x42, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x43] = IGarminTyp::polygon_property(0x43, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x44] = IGarminTyp::polygon_property(0x44, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x45] = IGarminTyp::polygon_property(0x45, Qt::NoPen,     "#0000ff", Qt::SolidPattern);
    polygonProperties[0x46] = IGarminTyp::polygon_property(0x46, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x47] = IGarminTyp::polygon_property(0x47, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x48] = IGarminTyp::polygon_property(0x48, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x49] = IGarminTyp::polygon_property(0x49, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x4a] = IGarminTyp::polygon_property(0x4a, "#000000",     Qt::NoBrush, Qt::NoBrush);
    polygonProperties[0x4b] = IGarminTyp::polygon_property(0x4b, "#000000",     Qt::NoBrush, Qt::NoBrush);
    polygonProperties[0x4c] = IGarminTyp::polygon_property(0x4c, Qt::NoPen,     "#f0e68c", Qt::SolidPattern);
    polygonProperties[0x4d] = IGarminTyp::polygon_property(0x4d, Qt::NoPen,     "#00ffff", Qt::SolidPattern);
    polygonProperties[0x4e] = IGarminTyp::polygon_property(0x4e, Qt::NoPen,     "#d3f5a5", Qt::SolidPattern);
    polygonProperties[0x4f] = IGarminTyp::polygon_property(0x4f, Qt::NoPen,     "#d3f5a5", Qt::SolidPattern);
    polygonProperties[0x50] = IGarminTyp::polygon_property(0x50, Qt::NoPen,     "#b7e999", Qt::SolidPattern);
    polygonProperties[0x51] = IGarminTyp::polygon_property(0x51, Qt::NoPen,     "#ffffff", Qt::SolidPattern);
    polygonProperties[0x52] = IGarminTyp::polygon_property(0x52, Qt::NoPen,     "#4aca4a", Qt::SolidPattern);
    polygonProperties[0x53] = IGarminTyp::polygon_property(0x53, Qt::NoPen,     "#bcedfa", Qt::SolidPattern);
    polygonProperties[0x54] = IGarminTyp::polygon_property(0x54, Qt::NoPen,     "#fde8d5", Qt::SolidPattern);
    polygonProperties[0x59] = IGarminTyp::polygon_property(0x59, Qt::NoPen,     "#0080ff", Qt::SolidPattern);
    polygonProperties[0x69] = IGarminTyp::polygon_property(0x69, Qt::NoPen,     "#0080ff", Qt::SolidPattern);

    polygonDrawOrder.clear();
    for(int i = 0; i < 0x80; i++)
    {
        polygonDrawOrder << (0x7F - i);
    }

    pointProperties.clear();

    if(useTyp)
    {
        readTYP();
    }
}


void CMapTDB::registerDEM(CMapDEM& dem)
{
    if(pjsrc == 0)
    {
        dem.deleteLater();
        throw tr("No basemap projection. That shouldn't happen.");
    }

    char * ptr = dem.getProjection();
    qDebug() << "Reproject map to:" << ptr;

    if(QString(ptr).contains("longlat"))
    {
        lon_factor =   PI / pow(2.0, 23);
        lat_factor = - PI / pow(2.0, 23);
        qDebug() << "set correction factor to" << lon_factor << lat_factor;
    }

    if(strlen(ptr) == 0)
    {
        return;
    }
    pj_free(pjsrc);
    pjsrc = pj_init_plus(ptr);
    //     if(ptr) free(ptr);
}


void CMapTDB::resize(const QSize& s)
{
    IMap::resize(s);
    topLeftInfo     = QPoint(size.width() - TEXTWIDTH - 10 , 10);
    setFastDraw();
}


bool CMapTDB::eventFilter(QObject * watched, QEvent * event)
{

    if(parent() == watched && event->type() == QEvent::MouseMove && !doFastDraw)
    {
        QMouseEvent * e = (QMouseEvent*)event;

        pointFocus = e->pos();

        QMultiMap<QString, QString> dict;
        getInfoPoints(pointFocus, dict);
        getInfoPois(pointFocus, dict);
        getInfoPolygons(pointFocus, dict);
        getInfoPolylines(pointFocus, dict);

        QString key;
        QStringList keys = dict.uniqueKeys();
        qSort(keys);

        infotext.clear();
        foreach(key,keys)
        {
            infotext += "<b>" + key + ":</b><br/>";
            const QStringList& values = dict.values(key).toSet().toList();
            infotext += values.join("<br/>");
            infotext += "<br/>";
        }

        if(!doFastDraw) emit sigChanged();

    }

    return IMap::eventFilter(watched, event);
}


void CMapTDB::readTDB(const QString& filename)
{
    QByteArray  data;
    QFile       file(filename);
    QFileInfo   finfo(filename);

    file.open(QIODevice::ReadOnly);
    data = file.readAll();
    file.close();

    quint8 * const pRawData = (quint8*)data.data();
    tdb_hdr_t * pRecord     = (tdb_hdr_t*)pRawData;

    quint32 basemapId   = 0;
    bool    tainted     = false;

    while((quint8*)pRecord < pRawData + data.size())
    {
        pRecord->size = gar_load(uint16_t,pRecord->size);
        switch(pRecord->type)
        {
            case 0x50:           // product name
            {
                tdb_product_t * p = (tdb_product_t*)pRecord;
                name = (char*)p->name;
            }
            break;

            case 0x42:           // basemap
            {
                tdb_map_t * p   = (tdb_map_t*)pRecord;

                basemapId       = gar_load(uint32_t,p->id);
                double n        = GARMIN_RAD((gar_load(int32_t,p->north) >> 8) & 0x00FFFFFF);
                double e        = GARMIN_RAD((gar_load(int32_t,p->east) >> 8)  & 0x00FFFFFF);
                double s        = GARMIN_RAD((gar_load(int32_t,p->south) >> 8) & 0x00FFFFFF);
                double w        = GARMIN_RAD((gar_load(int32_t,p->west) >> 8)  & 0x00FFFFFF);

                if(north < n) north = n;
                if(east  < e) east  = e;
                if(south > s) south = s;
                if(west  > w) west  = w;

                QSettings cfg;
                cfg.beginGroup("garmin/maps");
                cfg.beginGroup(name);
                basemap = cfg.value("basemap","").toString();
                mapkey  = cfg.value("key","").toString();
                cfg.endGroup();
                cfg.endGroup();

                QFileInfo basemapFileInfo(basemap);
                if(!basemapFileInfo.isFile())
                {

                    qApp->changeOverrideCursor(Qt::ArrowCursor);
                    QString filename = QFileDialog::getOpenFileName( 0, tr("Select Base Map for ") + name
                        ,finfo.dir().path()
                        ,"Map File (*.img)"
                        ,0
                        , QFileDialog::DontUseNativeDialog
                        );
                    qApp->restoreOverrideCursor();

                    if(filename.isEmpty())
                    {
                        deleteLater();
                        return;
                    }

                    cfg.beginGroup("garmin/maps");
                    cfg.beginGroup(name);
                    cfg.setValue("basemap",filename);
                    cfg.endGroup();
                    cfg.endGroup();

                    basemap = filename;
                }

                area = QRectF(QPointF(west, north), QPointF(east, south));

            }
            break;

            case 0x4C:           // map tiles
            {
                tdb_map_t * p = (tdb_map_t*)pRecord;
                p->id = gar_load(uint32_t,p->id);
                if(p->id == basemapId) break;

                QString tilename = QString::fromLatin1(p->name);
                // produce a unique key form the tile name and it's ID. Some tiles
                // might have the same name but never the same ID
                //QString key = QString("%1 %2").arg(tilename).arg(p->id,8,10,QChar('0'));
                QString key = QString("%1").arg(p->id,8,10,QChar('0'));

                tile_t& tile    = tiles[key];
                tile.id         = p->id;
                tile.key        = key;
                tile.name       = tilename;
                tile.file.sprintf("%08i.img",p->id);
                tile.file = finfo.dir().filePath(tile.file);

                //                 qDebug() << tile.file;

                tile.north  = GARMIN_RAD((gar_load(int32_t,p->north) >> 8) & 0x00FFFFFF);
                tile.east   = GARMIN_RAD((gar_load(int32_t,p->east) >> 8)  & 0x00FFFFFF);
                tile.south  = GARMIN_RAD((gar_load(int32_t,p->south) >> 8) & 0x00FFFFFF);
                tile.west   = GARMIN_RAD((gar_load(int32_t,p->west) >> 8)  & 0x00FFFFFF);
                tile.area   = QRectF(QPointF(tile.west, tile.north), QPointF(tile.east, tile.south));

                tile.defAreaU << tile.west << tile.east << tile.east << tile.west;
                tile.defAreaV << tile.north << tile.north << tile.south << tile.south;
                tile.defArea  << QPointF(tile.west, tile.north) << QPointF(tile.east, tile.north) << QPointF(tile.east, tile.south) << QPointF(tile.west, tile.south);

                tile.memSize = 0;
                tdb_map_size_t * s = (tdb_map_size_t*)(p->name + tilename.size() + 1);

                for(quint16 i=0; i < s->count; ++i)
                {
                    tile.memSize += gar_load(uint32_t,s->sizes[i]);
                }

//                try
//                {
//                    tile.img = new CGarminTile(this);
//                    tile.img->readBasics(tile.file);
//                }
//                catch(CGarminTile::exce_t e)
//                {
//
//                    if(e.err == CGarminTile::errLock)
//                    {
//                        if(!mapkey.isEmpty()) break;
//
//                        QMessageBox::warning(0,tr("Error"),e.msg,QMessageBox::Abort,QMessageBox::Abort);
//                        // help is on the way!!!
//                        mapkey = QInputDialog::getText(0,tr("However ...")
//                            ,tr("<p><b>However ...</b></p>"
//                            "<p>as I can read the basemap, and the information from the *tdb file,<br/>"
//                            "I am able to let you select the map tiles for upload. To do this I<br/>"
//                            "need the unlock key (25 digits) for this map, as it has to be uploaded<br/>"
//                            "to the unit together with the map.</p>"
//                            ));
//                        // no money, no brother, no sister - no key
//                        if(mapkey.isEmpty())
//                        {
//                            deleteLater();
//                            return;
//                        }
//
//                        QSettings cfg;
//                        cfg.beginGroup("garmin/maps");
//                        cfg.beginGroup(name);
//                        cfg.setValue("key",mapkey);
//                        cfg.endGroup();
//                        cfg.endGroup();
//
//                    }
//                    else if(CGarminTile::errFormat)
//                    {
//                        if(!tainted)
//                        {
//                            QMessageBox::warning(0, tr("Error")
//                                , tr("<p>Failed to load file:</p>"
//                                "<p>%1</p>"
//                                "<p>However, if the basemap is still old format I am able to let you select the map tiles for upload</p>"
//                                ).arg(e.msg)
//                                ,QMessageBox::Ok,QMessageBox::Ok);
//                            tainted = true;
//                        }
//                        delete tile.img;
//                        tile.img = 0;
//                    }
//                    else
//                    {
//                        if(!tainted)
//                        {
//                            QMessageBox::warning(0,tr("Error"),e.msg,QMessageBox::Ok,QMessageBox::Ok);
//                            tainted = true;
//                        }
//                        delete tile.img;
//                        tile.img = 0;
//                    }
//                }
            }
            break;

            case 0x44:
            {
                QString str;
                QTextStream out(&str,QIODevice::WriteOnly);

                out << "<h1>" << name << "</h1>" << endl;

                tdb_copyrights_t * p = (tdb_copyrights_t*)pRecord;
                tdb_copyright_t  * c = &p->entry;
                while((void*)c < (void*)((quint8*)p + gar_load(uint16_t,p->size) + 3))
                {

                    if(c->type != 0x07)
                    {
                        out << c->str << "<br/>" << endl;
                    }
                    c = (tdb_copyright_t*)((quint8*)c + 4 + strlen(c->str) + 1);
                }

                copyright += str;

                //                 qDebug() << "copyright:" << copyright;
            }
            break;
        }

        pRecord = (tdb_hdr_t*)((quint8*)pRecord + pRecord->size + sizeof(tdb_hdr_t));
    }
}

void CMapTDB::readTile(tile_t& tile)
{
    if(tile.img)
    {
        return;
    }

    try
    {
        tile.img = new CGarminTile(this);
        tile.img->readBasics(tile.file);
    }
    catch(CGarminTile::exce_t e)
    {

        if(e.err == CGarminTile::errLock)
        {
            if(!mapkey.isEmpty()) return;

            QMessageBox::warning(0,tr("Error"),e.msg,QMessageBox::Abort,QMessageBox::Abort);
            // help is on the way!!!
            mapkey = QInputDialog::getText(0,tr("However ...")
                ,tr("<p><b>However ...</b></p>"
                "<p>as I can read the basemap, and the information from the *tdb file,<br/>"
                "I am able to let you select the map tiles for upload. To do this I<br/>"
                "need the unlock key (25 digits) for this map, as it has to be uploaded<br/>"
                "to the unit together with the map.</p>"
                ));
            // no money, no brother, no sister - no key
            if(mapkey.isEmpty())
            {
                deleteLater();
                return;
            }

            QSettings cfg;
            cfg.beginGroup("garmin/maps");
            cfg.beginGroup(name);
            cfg.setValue("key",mapkey);
            cfg.endGroup();
            cfg.endGroup();

        }
        else if(CGarminTile::errFormat)
        {
            if(!tainted)
            {
                QMessageBox::warning(0, tr("Error")
                    , tr("<p>Failed to load file:</p>"
                    "<p>%1</p>"
                    "<p>However, if the basemap is still old format I am able to let you select the map tiles for upload</p>"
                    ).arg(e.msg)
                    ,QMessageBox::Ok,QMessageBox::Ok);
                tainted = true;
            }
            delete tile.img;
            tile.img = 0;
        }
        else
        {
            if(!tainted)
            {
                QMessageBox::warning(0,tr("Error"),e.msg,QMessageBox::Ok,QMessageBox::Ok);
                tainted = true;
            }
            delete tile.img;
            tile.img = 0;
        }
    }
}


bool CMapTDB::processPrimaryMapData()
{
    try
    {
        baseimg = new CGarminTile(this);
        baseimg->readBasics(basemap);
    }
    catch(CGarminTile::exce_t e)
    {
        // no basemap? bad luck!
        QMessageBox::warning(0,tr("Error"),e.msg,QMessageBox::Ok,QMessageBox::NoButton);
        deleteLater();
        return false;
    }

    qDebug() << "name:\t\t" << name;
    qDebug() << "basemap:\t" << basemap;
    qDebug() << "dimensions:\t" << "N" << north << "E" << east << "S" << south << "W" << west;

    const QMap<QString,CGarminTile::subfile_desc_t>& subfiles            = baseimg->getSubFiles();
    QMap<QString,CGarminTile::subfile_desc_t>::const_iterator subfile    = subfiles.begin();
    quint8 fewest_map_bits = 0xFF;

    /* Put here so the submap check doesn't do the basemap again. */
    QMap<QString,CGarminTile::subfile_desc_t>::const_iterator basemap_subfile;

    /* Find best candidate for basemap. */
    while (subfile != subfiles.end())
    {
        QVector<CGarminTile::maplevel_t>::const_iterator maplevel = subfile->maplevels.begin();
        /* Skip any upper levels that contain no real data. */
        while (!maplevel->inherited)
        {
            ++maplevel;
        }
        /* Check for the least detailed map. */
        if (maplevel->bits < fewest_map_bits)
        {
            fewest_map_bits = maplevel->bits;
            basemap_subfile = subfile;
        }
        ++subfile;
    }

    /* Add all basemap levels to the list. */
    quint8 largestBitsBasemap = 0;
    QVector<CGarminTile::maplevel_t>::const_iterator maplevel = basemap_subfile->maplevels.begin();
    while(maplevel != basemap_subfile->maplevels.end())
    {
        if (!maplevel->inherited)
        {
            map_level_t ml;
            ml.bits  = maplevel->bits;
            ml.level = maplevel->level;
            ml.useBaseMap = true;
            maplevels << ml;

            if(ml.bits > largestBitsBasemap) largestBitsBasemap = ml.bits;
        }
        ++maplevel;
    }

    if(!tiles.isEmpty())
    {
        CGarminTile * img = 0;
        QMap<QString,tile_t>::iterator tile = tiles.begin();
        while(tile != tiles.end())
        {
            readTile(*tile);
            img = tile->img;
            if(img) break;
            ++tile;
        }
        if(img)
        {
            const QMap<QString,CGarminTile::subfile_desc_t>& subfiles = img->getSubFiles();
            QMap<QString,CGarminTile::subfile_desc_t>::const_iterator subfile = subfiles.begin();
            /*
             * Query all subfiles for possible maplevels.
             * Exclude basemap to avoid polution.
             */
            while (subfile != subfiles.end())
            {
                QVector<CGarminTile::maplevel_t>::const_iterator maplevel = subfile->maplevels.begin();

                /* Skip basemap. */
                if (subfile == basemap_subfile)
                {
                    ++subfile;
                    continue;
                }
                while (maplevel != subfile->maplevels.end())
                {
                    if (!maplevel->inherited && (maplevel->bits > largestBitsBasemap))
                    {
                        map_level_t ml;
                        ml.bits  = maplevel->bits;
                        ml.level = maplevel->level;
                        ml.useBaseMap = false;
                        maplevels << ml;
                    }
                    ++maplevel;
                }
                ++subfile;
            }

            //             isTransparent = img->isTransparent();
        }
        /* Sort all entries, note that stable sort should insure that basemap is preferred when available. */
        qStableSort(maplevels.begin(), maplevels.end(), map_level_t::GreaterThan);
        /* Delete any duplicates for obvious performance reasons. */
        QVector<map_level_t>::iterator where;
        where = std::unique(maplevels.begin(), maplevels.end());
        maplevels.erase(where, maplevels.end());
    }

#ifdef DEBUG_SHOW_MAPLEVELS
    for(int i=0; i < maplevels.count(); ++i)
    {
        map_level_t& ml = maplevels[i];
        qDebug() << ml.bits << ml.level << ml.useBaseMap;
    }
#endif

    // read basemap for tile boundary polygons
    // Search all basemap levels for tile boundaries.
    // More detailed polygons will overwrite least detailed ones
    polygons.clear();
    for(int i=0; i < maplevels.count(); ++i)
    {
        if(!maplevels[i].useBaseMap) break;

        baseimg->loadPolygonsOfType(polygons, 0x4a, maplevels[i].level);

        polytype_t::iterator item = polygons.begin();
        while (item != polygons.end())
        {
            if((item->labels.size() > 1) && tiles.contains(item->labels[1]))
            {
                tile_t& tile = tiles[item->labels[1]];

                double * u = item->u.data();
                double * v = item->v.data();
                int N      = item->u.size();

                tile.defArea.clear();

                for(int n = 0; n < N; ++n, ++u, ++v)
                {
                    tile.defArea << QPointF(*u, *v);
                }

                tile.defAreaU = item->u;
                tile.defAreaV = item->v;

            }
            ++item;
        }
    }

    return true;
}


void CMapTDB::convertPt2M(double& u, double& v)
{
    XY pt = topLeft;
    pj_transform(pjtar,pjsrc,1,0,&pt.u,&pt.v,0);

    u = pt.u + u * zoomFactor * lon_factor;
    v = pt.v + v * zoomFactor * lat_factor;
}


void CMapTDB::convertM2Pt(double& u, double& v)
{
    XY pt = topLeft;
    pj_transform(pjtar,pjsrc,1,0,&pt.u,&pt.v,0);

    u = (u - pt.u) / (zoomFactor * lon_factor);
    v = (v - pt.v) / (zoomFactor * lat_factor);
}


void CMapTDB::convertM2Pt(double* u, double* v, int n)
{
    XY pt = topLeft;
    pj_transform(pjtar,pjsrc,1,0,&pt.u,&pt.v,0);

    for(int i = 0; i < n; ++i, ++u, ++v)
    {
        *u = (*u - pt.u) / (zoomFactor * lon_factor);
        *v = (*v - pt.v) / (zoomFactor * lat_factor);
    }
};

void CMapTDB::convertRad2Pt(double* u, double* v, int n)
{
    if(pjsrc == 0)
    {
        return;
    }

    pj_transform(pjtar,pjsrc,n,0,u,v,0);
    convertM2Pt(u,v,n);
}


void CMapTDB::move(const QPoint& old, const QPoint& next)
{
    XY p2 = topLeft;
    IMap::convertRad2Pt(p2.u, p2.v);

    // move top left point by difference
    p2.u += old.x() - next.x();
    p2.v += old.y() - next.y();

    // convert back to new top left geo coordinate
    convertPt2Rad(p2.u, p2.v);
    topLeft = p2;

    needsRedraw = true;
    setFastDraw();
    emit sigChanged();

    setAngleNorth();
}


void CMapTDB::zoom(bool zoomIn, const QPoint& p0)
{
    XY p1;

    needsRedraw = true;

    // convert point to geo. coordinates
    p1.u = p0.x();
    p1.v = p0.y();
    convertPt2Rad(p1.u, p1.v);

    zoomidx += zoomIn ? +1 : -1;
    // sigChanged will be sent at the end of this function
    blockSignals(true);
    zoom(zoomidx);

    // convert geo. coordinates back to point
    IMap::convertRad2Pt(p1.u, p1.v);

    XY p2 = topLeft;
    IMap::convertRad2Pt(p2.u, p2.v);

    // move top left point by difference point befor and after zoom
    p2.u += p1.u - p0.x();
    p2.v += p1.v - p0.y();

    // convert back to new top left geo coordinate
    convertPt2Rad(p2.u, p2.v);
    topLeft = p2;

    blockSignals(false);
    emit sigChanged();

    //     qDebug() << "maplevel" /*<< mapLevelMap << "(" << mapLevelOvl << ")"*/ << "bits" << scales[zoomidx].bits;
}


void CMapTDB::zoom(double lon1, double lat1, double lon2, double lat2)
{
    double u[3];
    double v[3];
    double dU, dV;

    needsRedraw = true;

    u[0] = lon1;
    v[0] = lat1;
    u[1] = lon2;
    v[1] = lat1;
    u[2] = lon1;
    v[2] = lat2;

    pj_transform(pjtar, pjsrc,3,0,u,v,0);
    dU = u[1] - u[0];
    dV = v[2] - v[0];


    for(int i = MAX_IDX_ZOOM; i >= MIN_IDX_ZOOM; --i)
    {

        double z    = scales[i].scale;
        double pxU  = dU / (+1.0 * z);
        double pxV  = dV / (-1.0 * z);

        if(isLonLat()){
            pxU /= lon_factor;
            pxV /= lat_factor;
        }

        if((fabs(pxU) < size.width()) && (fabs(pxV) < size.height()))
        {
            zoomFactor  = z;
            zoomidx     = i;
            double u = lon1 + (lon2 - lon1)/2;
            double v = lat1 + (lat2 - lat1)/2;
            IMap::convertRad2Pt(u,v);
            move(QPoint(u,v), rect.center());
            return;
        }
    }
}


void CMapTDB::zoom(qint32& level)
{
    needsRedraw = true;

    zoomidx = level;
    if(zoomidx < MIN_IDX_ZOOM) zoomidx = MIN_IDX_ZOOM;
    if(zoomidx > MAX_IDX_ZOOM) zoomidx = MAX_IDX_ZOOM;
    zoomFactor = scales[zoomidx].scale;

    qDebug() << zoomidx << zoomFactor << scales[zoomidx].bits << scales[zoomidx].scale << scales[zoomidx].label;

    setFastDraw();

    emit sigChanged();
}


void CMapTDB::dimensions(double& lon1, double& lat1, double& lon2, double& lat2)
{
    lon1 = west;
    lat1 = north;
    lon2 = east;
    lat2 = south;
}


void CMapTDB::getArea_n_Scaling(XY& p1, XY& p2, float& my_xscale, float& my_yscale)
{
    p1 = topLeft;
    p2 = bottomRight;

    XY p3;
    p3.u = p1.u;
    p3.v = p2.v;

    my_xscale = zoomFactor * lon_factor;
    my_yscale = zoomFactor * lat_factor;

}


void CMapTDB::draw(QPainter& p)
{
    bottomRight.u = size.width();
    bottomRight.v = size.height();
    convertPt2Rad(bottomRight.u, bottomRight.v);

    if((bottomRight.u < 0) && (bottomRight.u < topLeft.u))
    {
        bottomRight.u = 180 * DEG_TO_RAD + (180 * DEG_TO_RAD + bottomRight.u );
    }

    if((topLeft.u > 0) && (topLeft.u > bottomRight.u))
    {
        topLeft.u = -180 * DEG_TO_RAD - (180 * DEG_TO_RAD - topLeft.u);
    }

    // render map if necessary
    if(needsRedraw)
    {
        draw();
    }
    // copy internal buffer to paint device
    p.drawImage(0,0,buffer);

    // render overlay
    if(!ovlMap.isNull() && !doFastDraw)
    {
        ovlMap->draw(size, needsRedraw, p);
    }

    needsRedraw = false;

    if(!infotext.isEmpty() && info)
    {
        QFont f = p.font();
        f.setBold(false);
        f.setItalic(false);
        if(f.pointSize() < 8)
        {
            f.setPointSize(8);
        }
        info->setDefaultFont(f);
        info->setHtml(infotext);

        p.save();
        p.translate(topLeftInfo);

        QRectF rectInfo(QPointF(0,0), info->size());
        rectInfo.adjust(0,0,4,4);
        rectInfo.moveTopLeft(QPointF(-2,-2));

        p.setPen(Qt::black);
        p.setBrush(QColor(0xff, 0xff, 0xcc, 0xE0));
        p.drawRect(rectInfo);
        info->drawContents(&p);

        p.restore();
    }

    p.drawPixmap(pointFocus - QPoint(5,5), QPixmap(":/icons/small_bullet_yellow.png"));

    if(doFastDraw) setFastDraw();
}


void CMapTDB::draw(const QSize& s, bool needsRedraw, QPainter& p)
{
    int i;

    if(s != size)
    {
        resize(s);
        needsRedraw = true;
        doFastDraw  = false;
    }

    if(needsRedraw)
    {
        float sx, sy;
        getArea_n_Scaling_fromBase(topLeft, bottomRight, sx, sy);

        zoomFactor = sx/lon_factor;
        lat_factor = sy/sx * lon_factor;

        for(i=0; i < MAX_IDX_ZOOM; ++i)
        {
            if(scales[i].scale <= sx) break;
        }

        zoomidx     = i;

        draw();

        // make map semi transparent
        quint32 * ptr  = (quint32*)buffer.bits();
        for(int i = 0; i < (buffer.numBytes()>>2); ++i)
        {
            if(*ptr & 0xFF000000)
            {
                *ptr = (*ptr & 0x00FFFFFF) | 0xB0000000;
            }
            ++ptr;
        }
    }

    p.drawImage(0,0,buffer);

    if(ovlMap) ovlMap->draw(s, needsRedraw, p);
}


void CMapTDB::draw()
{
    buffer.fill(Qt::transparent);
    QPainter p(&buffer);

    QFont f = CResources::self().getMapFont();
    fm = QFontMetrics(f);

    p.setFont(f);
    p.setPen(Qt::black);
    p.setBrush(Qt::NoBrush);

    quint8 bits = scales[zoomidx].bits + detailsFineTune;
    QVector<map_level_t>::const_iterator maplevel = maplevels.end();
    do
    {
        --maplevel;
        if(bits >= maplevel->bits) break;
    } while(maplevel != maplevels.begin());

    QRectF viewport(QPointF(topLeft.u, topLeft.v), QPointF(bottomRight.u, bottomRight.v));
    polygons.clear();
    polylines.clear();
    pois.clear();
    points.clear();
    labels.clear();

    if(maplevel->useBaseMap)
    {
        baseimg->loadVisibleData(doFastDraw, polygons, polylines, points, pois, maplevel->level, viewport);
    }
    else
    {
        QMap<QString,tile_t>::iterator tile = tiles.begin();
        while(tile != tiles.end())
        {

            if(tile->area.intersects(viewport))
            {
                readTile(*tile);
                if(tile->img)
                {
                    tile->img->loadVisibleData(doFastDraw, polygons, polylines, points, pois, maplevel->level, viewport);
                }
            }
            ++tile;
        }
    }

    p.setRenderHint(QPainter::Antialiasing,!doFastDraw);

    if(/*!doFastDraw && */!isTransparent) {
    drawPolygons(p, polygons);
}


// needs to be removed
QPen pen(Qt::red, 30);
pen.setCapStyle(Qt::RoundCap);
pen.setJoinStyle(Qt::RoundJoin);
p.setPen(pen);
polytype_t::iterator item = query1.begin();
while(item != query1.end())
{
    QVector<double> lon = item->u;
    QVector<double> lat = item->v;
    double * u      = lon.data();
    double * v      = lat.data();
    const int size  = lon.size();

    convertRad2Pt(u,v,size);
    QPolygonF line(size);

    for(int i = 0; i < size; ++i)
    {
        line[i].setX(*u++);
        line[i].setY(*v++);
    }

    p.drawPolyline(line);

    ++item;
}


////////////////////////

if(!doFastDraw)
{
    drawPolylines(p, polylines);
    // needs to be removed
    p.setPen(QColor("#FFB000"));
    p.setBrush(QColor("#FFB000"));
    pointtype_t::iterator item = query2.begin();
    while(item != query2.end())
    {
        double u = item->lon;
        double v = item->lat;
        IMap::convertRad2Pt(u,v);
        p.drawEllipse(u - 16, v - 16, 30, 30);
        ++item;
    }
    ////////////////////////

    drawPoints(p, points);
    drawPois(p, pois);
    drawText(p);
    drawLabels(p, labels);

}


p.setRenderHint(QPainter::Antialiasing,false);
}


#define STREETNAME_THRESHOLD 5.0
void CMapTDB::drawLine(QPainter& p, CGarminPolygon& l, IGarminTyp::polyline_property& property, QFontMetricsF& metrics, QFont& font)
{
    QPolygonF line;
    double * u          = l.u.data();
    double * v          = l.v.data();
    const int size      = l.u.size();
    const int lineWidth = p.pen().width();

    if(size < 2)
    {
        return;
    }

    convertRad2Pt(u,v,size);

    line.clear();
    line.resize(size);

    for(int i = 0; i < size; ++i)
    {
        line[i].setX(*u++);
        line[i].setY(*v++);
    }

    if (zoomFactor < STREETNAME_THRESHOLD && property.labelType != IGarminTyp::eNone)
    {
        collectText(l, line, font, metrics, lineWidth);
    }

    p.drawPolyline(line);
}


void CMapTDB::drawLine(QPainter& p, CGarminPolygon& l)
{
    QPolygonF line;
    double * u          = l.u.data();
    double * v          = l.v.data();
    const int size      = l.u.size();

    if(size < 2)
    {
        return;
    }

    line.clear();
    line.resize(size);

    for(int i = 0; i < size; ++i)
    {
        line[i].setX(*u++);
        line[i].setY(*v++);
    }

    p.drawPolyline(line);
}


void CMapTDB::drawPolylines(QPainter& p, polytype_t& lines)
{
    textpaths.clear();
    QFont font = CResources::self().getMapFont();

    int fontsize = 6 + 3.0/zoomFactor;
    font.setPixelSize(fontsize);
    font.setBold(false);
    QFontMetricsF metrics(font);

    QList<quint32>keys = polylineProperties.keys();
    qSort(keys);
    quint32 type;

    foreach(type, keys)
    {
        IGarminTyp::polyline_property& property = polylineProperties[type];

        if(property.hasPixmap)
        {
            QImage pixmap = nightView ? property.imgNight : property.imgDay;
            const double w          = pixmap.width();
            const double h          = pixmap.height();

            polytype_t::iterator item = lines.begin();
            while(item != lines.end())
            {
                if(item->type == type)
                {
                    QPolygonF line;

                    double * u      = item->u.data();
                    double * v      = item->v.data();
                    const int size  = item->u.size();

                    if(size < 2)
                    {
                        ++item;
                        continue;
                    }

                    convertRad2Pt(u,v,size);

                    line.clear();
                    line.resize(size);

                    QVector<double> lengths;
                    double u1, u2, v1, v2;
                    QPainterPath path;
                    double segLength, totalLength = 0;
                    int i;

                    u1 = u[0];
                    v1 = v[0];
                    line[0].setX(u1);
                    line[0].setY(v1);

                    for(i = 1; i < size; ++i)
                    {
                        u2 = u[i];
                        v2 = v[i];

                        line[i].setX(u2);
                        line[i].setY(v2);

                        segLength    = sqrt((u2 - u1) * (u2 - u1) + (v2 - v1) * (v2 - v1));
                        totalLength += segLength;
                        lengths << segLength;

                        u1 = u2;
                        v1 = v2;
                    }

                    if (zoomFactor < STREETNAME_THRESHOLD && property.labelType != IGarminTyp::eNone)
                    {
                        collectText((*item), line, font, metrics, h);
                    }

                    path.addPolygon(line);
                    const int nLength = lengths.count();

                    double curLength = 0;
                    for(int i = 0; i < nLength; ++i)
                    {
                        segLength = lengths[i];

                        //                         qDebug() << curLength << totalLength << curLength / totalLength;

                        QPointF p1      = path.pointAtPercent(curLength / totalLength);
                        QPointF p2      = path.pointAtPercent((curLength + segLength) / totalLength);
                        double angle    = atan((p2.y() - p1.y()) / (p2.x() - p1.x())) * 180 / PI;

                        double l = 0;
                        QRectF r = pixmap.rect();

                        if(p2.x() - p1.x() < 0)
                        {
                            angle += 180;
                        }

                        p.save();
                        p.translate(p1);
                        p.rotate(angle);
                        p.translate(0,-h/2);

                        while(l < segLength)
                        {
                            if((segLength - l) < w)
                            {
                                r.setRight(segLength - l);
                                p.setClipRect(r, Qt::ReplaceClip);
                            }
                            p.drawImage(0,0,pixmap);
                            p.translate(w,0);
                            l += w;
                        }
                        p.restore();
                        curLength += segLength;
                    }
                }
                ++item;
            }
        }
        else
        {
            if(property.hasBorder)
            {
                // draw background line 1st
                p.setPen(nightView ? property.penBorderNight : property.penBorderDay);
                polytype_t::iterator item = lines.begin();
                while(item != lines.end())
                {
                    if(item->type == type)
                    {
                        drawLine(p, *item, property, metrics, font);
                    }
                    ++item;
                }
                // draw foreground line 2nd
                p.setPen(nightView ? property.penLineNight : property.penLineDay);
                item = lines.begin();
                while(item != lines.end())
                {
                    if(item->type == type)
                    {
                        drawLine(p, *item);
                    }
                    ++item;
                }
            }
            else
            {
                p.setPen(nightView ? property.penLineNight : property.penLineDay);
                polytype_t::iterator item = lines.begin();
                while(item != lines.end())
                {
                    if(item->type == type)
                    {
                        drawLine(p, *item, property, metrics, font);
                    }
                    ++item;
                }
            }
        }

    }
}


void CMapTDB::collectText(CGarminPolygon& item, QPolygonF& line,  QFont& font, QFontMetricsF metrics, qint32 lineWidth)
{

    QString str;
    if (!item.labels.isEmpty())
    {

        switch(item.type)
        {
            case 0x23:
            case 0x20:
            case 0x24:
            case 0x21:
            case 0x25:
            case 0x22:
            {
                QString unit;
                QString val = item.labels[0];
                IUnit::self().meter2elevation(val.toFloat() / 3.28084f, val, unit);
                str = QString("%1 %2").arg(val).arg(unit);
            }
            break;

            default:
                str = item.labels.join(" ").simplified();
        }
    }

    if(str.isEmpty()) return;

    textpath_t tp;
    tp.path.addPolygon(line);
    tp.font = font;
    tp.text = str;
    tp.lineWidth = lineWidth;

    QPointF p1 = line[0];
    const int size = line.size();
    for(int i = 1; i < size; ++i)
    {
        QPointF p2  = line[i];
        qreal dx    = p2.x() - p1.x();
        qreal dy    = p2.y() - p1.y();
        tp.lengths << sqrt(dx * dx + dy * dy);
        p1 = p2;
    }

    textpaths << tp;
}


void CMapTDB::drawText(QPainter& p)
{
    p.setPen(Qt::black);

    QVector<textpath_t>::iterator textpath = textpaths.begin();
    while(textpath != textpaths.end())
    {
        QFont& font         = textpath->font;
        QFontMetricsF fm(font);
        QPainterPath& path  = textpath->path;

        // get path length and string length
        qreal length        = fabs(path.length());
        qreal width         = fm.width(textpath->text);

        // adjust font size until string fits into polyline
        while(width > (length * 0.7))
        {
            font.setPixelSize(font.pixelSize() - 1);
            fm      = QFontMetricsF(font);
            width   = fm.width(textpath->text);

            if((font.pixelSize() < 8)) break;
        }

        // no way to draw a readable string - skip
        if((font.pixelSize() < 8))
        {
            ++textpath;
            continue;
        }

        // adjust exact offset to first half of segment
        const QVector<qreal>& lengths = textpath->lengths;
        const qreal ref = (length - width) / 2;
        qreal offset    = 0;

        for(int i = 0; i < lengths.size(); ++i)
        {
            const qreal d = lengths[i];

            if((offset + d/2) >= ref)
            {
                offset = ref;
                break;
            }
            if((offset + d) >= ref)
            {
                offset += d/2;
                break;
            }
            offset += d;
        }

        // get starting angle of first two letters
        QString& text   = textpath->text;
        qreal percent1  =  offset / length;
        qreal percent2  = (offset + fm.width(text.left(2))) / length;

        QPointF point1  = path.pointAtPercent(percent1);
        QPointF point2  = path.pointAtPercent(percent2);

        qreal angle     = atan((point2.y() - point1.y()) / (point2.x() - point1.x())) * 180 / PI;

        // flip path if string start is E->W direction
        // this helps, sometimes, in 50 % of the cases :)
        if(point2.x() - point1.x() < 0)
        {
            path    = path.toReversed();
        }

        p.setFont(textpath->font);

        // draw string letter by letter and adjust angle
        const int size = text.size();
        for(int i = 0; i < size; ++i)
        {

            percent1  =  offset / length;
            percent2  = (offset + fm.width(text[i])) / length;

            point1  = path.pointAtPercent(percent1);
            point2  = path.pointAtPercent(percent2);

            angle   = atan((point2.y() - point1.y()) / (point2.x() - point1.x())) * 180 / PI;

            if(point2.x() - point1.x() < 0)
            {
                angle += 180;
            }

            p.save();
            p.translate(point1);
            p.rotate(angle);
            if(textAboveLine)
            {
                p.translate(0, -(textpath->lineWidth + 2));
            }
            else
            {
                p.translate(0, fm.descent());
            }
            p.drawText(0,0,text.mid(i,1));
            p.restore();

            offset += fm.width(text[i]);
        }

        ++textpath;
    }

}


void CMapTDB::drawPolygons(QPainter& p, polytype_t& lines)
{
    quint32 type;
    int n;
    const int N = polygonDrawOrder.size();
    for(n = 0; n < N; ++n)
    {
        type = polygonDrawOrder[(N-1) - n];

        p.setPen(polygonProperties[type].pen);
        p.setBrush(nightView ? polygonProperties[type].brushNight : polygonProperties[type].brushDay);

        polytype_t::iterator item = lines.begin();
        while (item != lines.end())
        {

            if(item->type != type)
            {
                ++item;
                continue;
            }

            double * u      = item->u.data();
            double * v      = item->v.data();
            const int size  = item->u.size();

            convertRad2Pt(u,v,size);

            QPolygonF line(size);
            for(int i = 0; i < size; ++i)
            {
                line[i].setX(*u++);
                line[i].setY(*v++);
            }

            p.drawPolygon(line);

            if(!polygonProperties[type].known) qDebug() << "unknown polygon" << hex << type;
            ++item;
        }
    }

}


void CMapTDB::drawPoints(QPainter& p, pointtype_t& pts)
{

    pointtype_t::iterator pt = pts.begin();
    while(pt != pts.end())
    {

        if((pt->type > 0x1600) && (zoomFactor > 5.0))
        {
            ++pt;
            continue;
        };

        IMap::convertRad2Pt(pt->lon, pt->lat);

        bool showLabel = true;
        if(pointProperties.contains(pt->type))
        {
            p.drawImage(pt->lon - 4, pt->lat - 4, nightView ? pointProperties[pt->type].imgNight : pointProperties[pt->type].imgDay);
            showLabel = pointProperties[pt->type].labelType != IGarminTyp::eNone;
        }
        else
        {
            p.drawPixmap(pt->lon - 4, pt->lat - 4, QPixmap(":/icons/small_bullet_blue.png"));
        }

        if(!pt->labels.isEmpty() && ((zoomFactor < 2) || (pt->type < 0x1600))  && poiLabels && showLabel)
        {

            // calculate bounding rectangle with a border of 2 px
            QRect rect = fm.boundingRect(pt->labels.join(" "));
            rect.adjust(0,0,4,4);
            rect.moveCenter(QPoint(pt->lon, pt->lat));

            // test rectangle for intersection with existng labels
            QVector<strlbl_t>::const_iterator label = labels.begin();
            while(label != labels.end())
            {
                if(label->rect.intersects(rect)) break;
                ++label;
            }

            // if no intersection was found, add label to list
            if(label == labels.end())
            {
                labels.push_back(strlbl_t());
                strlbl_t& strlbl = labels.last();
                strlbl.pt   = QPoint(pt->lon, pt->lat);
                strlbl.str  = pt->labels.join(" ");
                strlbl.rect = rect;
            }
        }
        ++pt;
    }
}


void CMapTDB::drawPois(QPainter& p, pointtype_t& pts)
{
    pointtype_t::iterator pt = pts.begin();
    while(pt != pts.end())
    {
        IMap::convertRad2Pt(pt->lon, pt->lat);

        bool showLabel = true;
        if(pointProperties.contains(pt->type))
        {
            p.drawImage(pt->lon - 4, pt->lat - 4, nightView ? pointProperties[pt->type].imgNight : pointProperties[pt->type].imgDay);
            showLabel = pointProperties[pt->type].labelType != IGarminTyp::eNone;
        }
        else
        {
            p.drawPixmap(pt->lon - 4, pt->lat - 4, QPixmap(":/icons/small_bullet_red.png"));
        }

        if(!pt->labels.isEmpty() && (zoomFactor < 2) && poiLabels && showLabel)
        {

            // calculate bounding rectangle with a border of 2 px
            QRect rect = fm.boundingRect(pt->labels.join(" "));
            rect.adjust(0,0,4,4);
            rect.moveCenter(QPoint(pt->lon, pt->lat));

            // test rectangle for intersection with existng labels
            QVector<strlbl_t>::const_iterator label = labels.begin();
            while(label != labels.end())
            {
                if(label->rect.intersects(rect)) break;
                ++label;
            }

            // if no intersection was found, add label to list
            if(label == labels.end())
            {
                labels.push_back(strlbl_t());
                strlbl_t& strlbl = labels.last();
                strlbl.pt   = QPoint(pt->lon, pt->lat);
                strlbl.str  = pt->labels.join(" ");
                strlbl.rect = rect;
            }
        }
        ++pt;
    }
}


void CMapTDB::drawLabels(QPainter& p, QVector<strlbl_t> lbls)
{
    QVector<strlbl_t>::const_iterator lbl = lbls.begin();
    while(lbl != lbls.end())
    {
        CCanvas::drawText(lbl->str, p, lbl->pt, Qt::black);
        ++lbl;
    }
}


void CMapTDB::getInfoPoints(const QPoint& pt, QMultiMap<QString, QString>& dict)
{
    pointtype_t::const_iterator point = points.begin();
    while(point != points.end())
    {
        QPoint x = pt - QPoint(point->lon, point->lat);
        if(x.manhattanLength() < 10)
        {
            if(point->labels.size())
            {
                dict.insert(tr("Point of Interest"),point->labels.join( ", " ) + QString(" (%1)").arg(point->type,2,16,QChar('0')));
            }
            else
            {
                if(pointProperties.contains(point->type))
                {
                    if(selectedLanguage != -1)
                    {
                        dict.insert(tr("Point of Interest"),pointProperties[point->type].strings[selectedLanguage] + QString(" (%1)").arg(point->type,2,16,QChar('0')));
                    }
                    else
                    {
                        dict.insert(tr("Point of Interest"), QString(" (%1)").arg(point->type,2,16,QChar('0')));
                    }
                }
                else
                {
                    dict.insert(tr("Point of Interest"), QString(" (%1)").arg(point->type,2,16,QChar('0')));
                }
            }
        }
        ++point;
    }

}


void CMapTDB::getInfoPois(const QPoint& pt, QMultiMap<QString, QString>& dict)
{
    pointtype_t::const_iterator point = pois.begin();
    while(point != pois.end())
    {
        QPoint x = pt - QPoint(point->lon, point->lat);
        if(x.manhattanLength() < 10)
        {
            if(point->labels.size())
            {
                dict.insert(tr("Point of Interest"),point->labels.join( ", " ) + QString(" (%1)").arg(point->type,2,16,QChar('0')));
            }
            else
            {
                if(pointProperties.contains(point->type))
                {
                    if(selectedLanguage != -1)
                    {
                        dict.insert(tr("Point of Interest"),pointProperties[point->type].strings[selectedLanguage] + QString(" (%1)").arg(point->type,2,16,QChar('0')));
                    }
                    else
                    {
                        dict.insert(tr("Point of Interest"), QString(" (%1)").arg(point->type,2,16,QChar('0')));
                    }
                }
                else
                {
                    dict.insert(tr("Point of Interest"), QString(" (%1)").arg(point->type,2,16,QChar('0')));
                }
            }
        }
        ++point;
    }
}


void CMapTDB::getInfoPolylines(QPoint& pt, QMultiMap<QString, QString>& dict)
{
    int i = 0;                   // index into poly line
    int len;                     // number of points in line
    XY p1, p2;                   // the two points of the polyline close to pt
    double dx,dy;                // delta x and y defined by p1 and p2
    double d_p1_p2;              // distance between p1 and p2
    double u;                    // ratio u the tangent point will divide d_p1_p2
    double x,y;                  // coord. (x,y) of the point on line defined by [p1,p2] close to pt
    double distance;             // the distance to the polyline
    double shortest;             // shortest distance sofar

    QPointF resPt = pt;
    QString key, value;
    quint32 type = 0;

    shortest = 50;

    polytype_t::const_iterator line = polylines.begin();
    while(line != polylines.end())
    {
        len = line->u.count();
        // need at least 2 points
        if(len < 2)
        {
            ++line;
            continue;
        }

        // see http://local.wasp.uwa.edu.au/~pbourke/geometry/pointline/
        for(i=1; i<len; ++i)
        {
            p1.u = line->u[i-1];
            p1.v = line->v[i-1];
            p2.u = line->u[i];
            p2.v = line->v[i];

            dx = p2.u - p1.u;
            dy = p2.v - p1.v;

            d_p1_p2 = sqrt(dx * dx + dy * dy);

            u = ((pt.x() - p1.u) * dx + (pt.y() - p1.v) * dy) / (d_p1_p2 * d_p1_p2);

            if(u < 0.0 || u > 1.0) continue;

            x = p1.u + u * dx;
            y = p1.v + u * dy;

            distance = sqrt((x - pt.x())*(x - pt.x()) + (y - pt.y())*(y - pt.y()));

            if(distance < shortest)
            {
                type = line->type;

                if(!line->labels.isEmpty())
                {
                    switch(type)
                    {
                                 // "Minor depht contour"
                        case 0x23:
                                 // "Minor land contour"
                        case 0x20:
                                 // "Intermediate depth contour",
                        case 0x24:
                                 // "Intermediate land contour",
                        case 0x21:
                                 // "Major depth contour",
                        case 0x25:
                                 // "Major land contour",
                        case 0x22:
                        {
                            QString unit;
                            QString val = line->labels[0];
                            IUnit::self().meter2elevation(val.toFloat() / 3.28084f, val, unit);
                            value = QString("%1 %2").arg(val).arg(unit);
                        }
                        break;

                        default:
                            value = line->labels.join(" ").simplified();
                    }
                }
                else
                {
                    value = "-";
                }
                resPt.setX(x);
                resPt.setY(y);
                shortest = distance;
            }

        }
        ++line;
    }

    if(selectedLanguage != -1)
    {
        key =  polylineProperties[type].strings[selectedLanguage];
    }

    if(!key.isEmpty())
    {
        dict.insert(key + QString("(%1)").arg(type,2,16,QChar('0')),value);
    }
    else
    {
        dict.insert(tr("Unknown") + QString("(%1)").arg(type,2,16,QChar('0')),value);
    }

    pt = resPt.toPoint();
}


void CMapTDB::getInfoPolygons(const QPoint& pt, QMultiMap<QString, QString>& dict)
{
    int     npol;
    int     i = 0, j = 0 ,c = 0;
    XY      p1, p2;              // the two points of the polyline close to pt
    double  x = pt.x();
    double  y = pt.y();
    QString value;

    polytype_t::const_iterator line = polygons.begin();
    while(line != polygons.end())
    {

        npol = line->u.count();
        if(npol > 2)
        {
            c = 0;
            // see http://local.wasp.uwa.edu.au/~pbourke/geometry/insidepoly/
            for (i = 0, j = npol-1; i < npol; j = i++)
            {
                p1.u = line->u[j];
                p1.v = line->v[j];
                p2.u = line->u[i];
                p2.v = line->v[i];

                if ((((p2.v <= y) && (y < p1.v))  || ((p1.v <= y) && (y < p2.v))) &&
                    (x < (p1.u - p2.u) * (y - p2.v) / (p1.v - p2.v) + p2.u))
                {
                    c = !c;
                }
            }

            if(c)
            {
                if(line->labels.isEmpty())
                {

                    if(selectedLanguage != -1 && polygonProperties[line->type].strings[selectedLanguage].size())
                    {
                        dict.insert(tr("Area"), polygonProperties[line->type].strings[selectedLanguage]  + QString(" (%1)").arg(line->type,2,16,QChar('0')));
                    }
                }
                else
                {
                    dict.insert(tr("Area"), line->labels.join(" ").simplified()  + QString(" (%1)").arg(line->type,2,16,QChar('0')));
                }
            }
        }
        ++line;
    }
}


void CMapTDB::select(IMapSelection& ms, const QRect& rect)
{
    CMapSelectionGarmin& sel = (CMapSelectionGarmin&)ms;

    double lon1 = rect.left();
    double lat1 = rect.top();
    convertPt2Rad(lon1, lat1);

    double lon2 = rect.right();
    double lat2 = rect.bottom();
    convertPt2Rad(lon2, lat2);

    QPolygonF poly;
    poly << QPointF(lon1, lat1) << QPointF(lon2, lat1) << QPointF(lon2, lat2) << QPointF(lon1, lat2);

    if(!sel.maps.contains(key))
    {
        CMapSelectionGarmin::map_t& m = sel.maps[key];
        m.unlockKey = mapkey;
        m.name      = name;
        m.typfile   = typfile;
        m.pid       = pid;
        m.fid       = fid;
    }
    QMap<QString, CMapSelectionGarmin::map_t>::iterator map = sel.maps.find(key);

    QMap<QString,tile_t>::iterator tile = tiles.begin();
    while(tile != tiles.end())
    {
        QPolygonF res = poly.intersected(tile->defArea);

        if(!res.isEmpty())
        {

            if(map->tiles.contains(tile->key))
            {
                map->tiles.remove(tile->key);
            }
            else
            {
                CMapSelectionGarmin::tile_t t;
                t.id        = tile->id;
                t.name      = tile->name;
                t.filename  = tile->file;
                t.u         = tile->defAreaU;
                t.v         = tile->defAreaV;
                t.memSize   = tile->memSize;
                t.area      = tile->area;
                t.pid       = pid;
                t.fid       = fid;
                map->tiles[tile->key] = t;
            }
        }

        ++tile;
    }

    sel.maps[key] = *map;
    sel.calcArea();

    quint32 memSize = sel.getMemSize();
    sel.description += QString("\nSize: %1 MB").arg(double(memSize) / (1024 * 1024), 0,'f',2);
    sel.description += QString("\nTiles: #%1").arg(sel.tilecnt);
}


void CMapTDB::readTYP()
{
    QFileInfo fi(filename);
    QDir path = fi.absoluteDir();
    QStringList filters;

    filters << "*.typ" << "*.TYP";
    QStringList typfiles = path.entryList(filters, QDir::Files);

    if(typfiles.isEmpty()) return;

#ifdef DBG
    QString f;
    foreach(f, typfiles)
    {

        typfile = path.absoluteFilePath(f);
        QFile file(path.absoluteFilePath(f));
#else
        typfile = path.absoluteFilePath(typfiles[0]);
        QFile file(path.absoluteFilePath(typfiles[0]));
#endif
        qDebug() << file.fileName();
        file.open(QIODevice::ReadOnly);

        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_4_5);
        in.setByteOrder( QDataStream::LittleEndian);

        /* Read typ file descriptor */
        quint16 descriptor;
        in >> descriptor;

        qDebug() << "descriptor" << hex << descriptor;
        switch(descriptor)
        {
            case 0x6E:
            {
                CGarminTypNT typ(0);
                typ.decode(in, polygonProperties, polylineProperties, polygonDrawOrder, pointProperties);

                quint8 lang;
                QSet<quint8> usedLanguages = typ.getLanguages();

                if(!usedLanguages.isEmpty())
                {
                    comboLanguages = new QComboBox(theMainWindow->getCanvas());
                    foreach(lang, usedLanguages)
                    {
                        comboLanguages->addItem(languages[lang],  lang);
                    }
                    connect(comboLanguages, SIGNAL(currentIndexChanged(int)), this, SLOT(slotLanguageChanged(int)));
                    theMainWindow->statusBar()->insertPermanentWidget(0,comboLanguages);

                    QSettings cfg;
                    cfg.beginGroup("garmin/maps");
                    cfg.beginGroup(name);
                    selectedLanguage  = cfg.value("selectedLanguage",usedLanguages.toList().first()).toInt();
                    cfg.endGroup();
                    cfg.endGroup();

                    comboLanguages->setCurrentIndex(comboLanguages->findData(selectedLanguage));

                }

                pid = typ.getPid();
                fid = typ.getFid();
                break;
            }

            default:
            case 0x5B:
            {
                CGarminTyp typ(0);
                typ.decode(in, polygonProperties, polylineProperties, polygonDrawOrder, pointProperties);

                quint8 lang;
                QSet<quint8> usedLanguages = typ.getLanguages();

                if(!usedLanguages.isEmpty())
                {
                    comboLanguages = new QComboBox(theMainWindow->getCanvas());
                    foreach(lang, usedLanguages)
                    {
                        comboLanguages->addItem(languages[lang],  lang);
                    }
                    connect(comboLanguages, SIGNAL(currentIndexChanged(int)), this, SLOT(slotLanguageChanged(int)));
                    theMainWindow->statusBar()->insertPermanentWidget(0,comboLanguages);

                    QSettings cfg;
                    cfg.beginGroup("garmin/maps");
                    cfg.beginGroup(name);
                    selectedLanguage  = cfg.value("selectedLanguage",usedLanguages.toList().first()).toInt();
                    cfg.endGroup();
                    cfg.endGroup();

                    comboLanguages->setCurrentIndex(comboLanguages->findData(selectedLanguage));

                }
                pid = typ.getPid();
                fid = typ.getFid();

                break;
            }

            //         default:
            //             qDebug() << "CMapTDB::readTYP() not a known typ file = " << descriptor;
            //             QMessageBox::warning(0, tr("Warning..."), tr("Unknown typ file format in '%1'. Use http://ati.land.cz/gps/typdecomp/editor.cgi to convert file to either old or NT format.").arg(typfile), QMessageBox::Abort, QMessageBox::Abort);
            //             return;
        }

        file.close();

#ifdef DBG
    }
#endif
}

#ifdef SQL_SEARCH_GARMIN
void CMapTDB::createSearchIndex(QObject * receiver, const char * slot)
{
    QStringList files;
    QMap<QString,tile_t>::const_iterator tile = tiles.begin();
    while(tile != tiles.end())
    {
        files << tile->file;
        ++tile;
    }
    connect(index, SIGNAL(sigProgress(const QString&, const int)), receiver, slot);
    index->create(files);
}


void CMapTDB::highlight(QVector<CGarminPolygon>& res)
{
    query1 = res;
    needsRedraw = true;
    emit sigChanged();
}


void CMapTDB::highlight(QVector<CGarminPoint>& res)
{
    query2 = res;
    needsRedraw = true;
    emit sigChanged();
}
#endif //SQL_SEARCH_GARMIN

void CMapTDB::config()
{
    CDlgMapTDBConfig dlg(this);
    dlg.exec();

    needsRedraw = true;
    if(comboLanguages)
    {
        delete comboLanguages;
        comboLanguages  = 0;
    }

    if(useTyp)
    {
        readTYP();
    }
    else
    {
        setup();
    }
    emit sigChanged();
}
