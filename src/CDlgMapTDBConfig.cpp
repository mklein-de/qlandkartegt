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

#include "CDlgMapTDBConfig.h"
#include "CMapTDB.h"

#include <QtGui>

#ifndef WIN32
#include <pwd.h>
#endif

const QString CDlgMapTDBConfig::text =  QObject::tr(""
"<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN' 'http://www.w3.org/TR/REC-html40/strict.dtd'>"
"<html>"
"   <head>"
"       <META HTTP-EQUIV='CACHE-CONTROL'' CONTENT='NO-CACHE'>"
"       <style type='text/css'>"
"           p, li { white-space: pre-wrap; }"
"           td {padding-top: 10px;}"
"       </style>"
"   </head>"
"   <body style=' font-family:'Sans'; font-size:9pt; font-weight:400; font-style:normal;'>"
"       <p>${copyright}</p>"
"       <h1>Map Levels</h1>"
"       <p>${maplevels}</p>"
"       <h1>Legend</h1>"
"       <h2>Lines</h2>"
"       <p>${legendlines}</p>"
"       <h2>Areas</h2>"
"       <p>${legendareas}</p>"
"       <h2>Points</h2>"
"       <p>${legendpoints}</p>"
"   </body>"
"</html>"
"");

CDlgMapTDBConfig::CDlgMapTDBConfig(CMapTDB * map)
: map(map)
{
    setupUi(this);

    QDir tempDir;
#ifndef Q_OS_WIN32
    const char *envCache = getenv("QLGT_LEGEND");

    if (envCache)
    {
        tempDir = envCache;
    }
    else
    {
        struct passwd * userInfo = getpwuid(getuid());
        tempDir = QDir::tempPath() + "/qlandkarteqt-" + userInfo->pw_name + "/legend/";
    }
#else
    tempDir = QDir::tempPath() + "/qlandkarteqt/cache/";
#endif
    tempDir.mkpath(tempDir.path());

    QString cpytext = text;
    cpytext = cpytext.replace("${copyright}", map->getCopyright());
    cpytext = cpytext.replace("${maplevels}", map->getMapLevelInfo());
    cpytext = cpytext.replace("${legendlines}", map->getLegendLines());
    cpytext = cpytext.replace("${legendareas}", map->getLegendArea());
    cpytext = cpytext.replace("${legendpoints}", map->getLegendPoints());

    webView->page()->settings()->clearMemoryCaches();

    QFile tmp(tempDir.filePath("legend.html"));
    tmp.open(QIODevice::WriteOnly);
    tmp.write(cpytext.toLocal8Bit());
    tmp.close();

    webView->load(QString("file://") + tempDir.filePath("legend.html"));

    connect(this, SIGNAL(accepted()), SLOT(deleteLater()));
    connect(this, SIGNAL(rejected()), SLOT(deleteLater()));
}


CDlgMapTDBConfig::~CDlgMapTDBConfig()
{
    qDebug() << "CDlgMapTDBConfig::~CDlgMapTDBConfig()";
}


void CDlgMapTDBConfig::accept()
{

    QDialog::accept();
}
