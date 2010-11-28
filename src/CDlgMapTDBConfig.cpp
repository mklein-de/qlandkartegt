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

static const QString text =  QObject::tr(""
"<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN' 'http://www.w3.org/TR/REC-html40/strict.dtd'>"
"<html>"
"   <head>"
"       <meta name='qrichtext' content='1' />"
"       <style type='text/css'>p, li { white-space: pre-wrap; }</style>"
"   </head>"
"   <body style=' font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;'>"
"       <p>${copyright}</p>"
"       <h1>Map Levels</h1>"
"       <p>${maplevels}</p>"
"   </body>"
"</html>"
"");

CDlgMapTDBConfig::CDlgMapTDBConfig(CMapTDB * map)
: map(map)
{
    setupUi(this);

    QString cpytext = text;
    cpytext = cpytext.replace("${copyright}", map->getCopyright());
    cpytext = cpytext.replace("${maplevels}", map->getMapLevelInfo());

    textEdit->setHtml(cpytext);

}


CDlgMapTDBConfig::~CDlgMapTDBConfig()
{

}


void CDlgMapTDBConfig::accept()
{

    QDialog::accept();
}
