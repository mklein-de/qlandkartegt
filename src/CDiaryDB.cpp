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

#include "CDiaryDB.h"
#include "CDiaryEditWidget.h"
#include "CTabWidget.h"
#include "CQlb.h"
#include "CGpx.h"

#include <QtGui>

CDiaryDB * CDiaryDB::m_self = 0;

CDiaryDB::CDiaryDB(QTabWidget * tb, QObject * parent)
: IDB(tb, parent)
, diary(this)
{
    m_self = this;
}


CDiaryDB::~CDiaryDB()
{

}


void CDiaryDB::openEditWidget()
{

    CTabWidget * tb = dynamic_cast<CTabWidget*>(tabbar);
    if(tb == 0) return;

    if(editWidget.isNull())
    {
        editWidget = new CDiaryEditWidget(diary.text(), tabbar);
        tb->addTab(editWidget,tr("Diary"));

    }
    else
    {
        diary.setText(editWidget->textEdit->toHtml());
        delete editWidget;

    }
}


void CDiaryDB::loadQLB(CQlb& qlb)
{
    QDataStream stream(&qlb.diary(),QIODevice::ReadOnly);
    stream >> diary;
    if(!editWidget.isNull())
    {
        editWidget->textEdit->setHtml(diary.text());
    }
    if(count())
    {
        emit sigChanged();
    }
}


void CDiaryDB::saveQLB(CQlb& qlb)
{
    if(!editWidget.isNull())
    {
        diary.setText(editWidget->textEdit->toHtml());
    }
    qlb << diary;
}


void CDiaryDB::clear()
{
    CTabWidget * tb = dynamic_cast<CTabWidget*>(tabbar);
    if(tb == 0) return;

    if(!editWidget.isNull())
    {
        delete editWidget;
    }
    diary = CDiary(this);
    emit sigChanged();
}


const QString CDiaryDB::getDiary()
{
    if(!editWidget.isNull())
    {
        diary.setText(editWidget->textEdit->toHtml());
    }
    return diary.text();
}


int CDiaryDB::count()
{
    if(!editWidget.isNull())
    {
        diary.setText(editWidget->textEdit->toHtml());
    }
    QTextBrowser browser;
    browser.setHtml(diary.text());
    return !browser.toPlainText().isEmpty();
}


void CDiaryDB::loadGPX(CGpx& gpx)
{
    if (gpx.version() == CGpx::qlVer_foreign)
        return;

    // QLandkarteGT file format v1.0 had more than one extensions
    // tags, so we have to scan all of them.  We can stop once we
    // found a diary tag below it.
    QDomElement extensions = gpx.firstChildElement("gpx").firstChildElement("extensions");
    while(!extensions.isNull())
    {
        QMap<QString,QDomElement> extensionsmap = CGpx::mapChildElements(extensions);
        const QDomElement dry = extensionsmap.value(gpx.version() == CGpx::qlVer_1_0?
            "diary":
        (CGpx::ql_ns + ":" + "diary"));
        if(!dry.isNull())
        {
            QString tmp = diary.text();
            tmp += dry.toElement().text();
            diary.setText(tmp);
            break;
        }
        extensions = extensions.nextSiblingElement("extensions");
    }

    if(count())
    {
        emit sigChanged();
    }
}


void CDiaryDB::saveGPX(CGpx& gpx)
{

    if (gpx.getExportFlag())
    {
        return;
    }
    if(!editWidget.isNull())
    {
        diary.setText(editWidget->textEdit->toHtml());
    }

    const QString diary_text = diary.text();
    if (diary_text.length() == 0)
    {
        return;
    }

    QDomElement root        = gpx.documentElement();
    QDomElement extensions  = gpx.getExtensions();
    QDomElement dry         = gpx.createElement("ql:diary");
    QDomText text           = gpx.createTextNode(diary_text);

    root.appendChild(extensions);
    extensions.appendChild(dry);
    dry.appendChild(text);
}
