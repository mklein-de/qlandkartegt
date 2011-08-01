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
#include "CTextEditWidget.h"
#include "CTabWidget.h"
#include "CQlb.h"
#include "CGpx.h"

#include <QtGui>

CDiaryDB * CDiaryDB::m_self = 0;

CDiaryDB::CDiaryDB(QTabWidget * tb, QObject * parent)
: IDB(tb, parent)
{
    m_self = this;

}

CDiaryDB::~CDiaryDB()
{

}

void CDiaryDB::loadQLB(CQlb& qlb, bool newKey)
{
    QDataStream stream(&qlb.diary(),QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_4_5);

    while(!stream.atEnd())
    {
        CDiary * diary = new CDiary(this);
        stream >> *diary;

        if(newKey)
        {
            diary->setKey(diary->getKey() + QString("%1").arg(QDateTime::currentDateTime().toTime_t()));
        }

        addDiary(diary,true, false);
    }

    if(qlb.diary().size())
    {
        emit sigChanged();
    }

}


void CDiaryDB::saveQLB(CQlb& qlb)
{
    foreach(CDiary* diary, diarys)
    {
        qlb << *diary;
    }
}


void CDiaryDB::clear()
{
    delDiarys(diarys.keys());
    emit sigChanged();
}

int CDiaryDB::count()
{
    return diarys.count();
}


void CDiaryDB::loadGPX(CGpx& gpx)
{
    if (gpx.version() == CGpx::qlVer_foreign)
    {
        return;
    }
}


void CDiaryDB::saveGPX(CGpx& gpx, const QStringList& keys)
{

    if (gpx.getExportMode() != CGpx::eQlgtExport)
    {
        return;
    }
}


void CDiaryDB::addDiary(CDiary * diary, bool silent, bool fromDB)
{
    CTabWidget * tb = dynamic_cast<CTabWidget*>(tabbar);
    if(tb == 0) return;

    delDiary(diary->getKey(), silent);
    diarys[diary->getKey()] = diary;

    diary->showEditWidget(tb,fromDB);

    connect(diary,SIGNAL(sigChanged()),SIGNAL(sigChanged()));
    if(!silent)
    {
        emit sigChanged();
        emit sigModified();
    }
}

void CDiaryDB::delDiary(const QString& key, bool silent)
{
    if(!diarys.contains(key)) return;
    diarys.take(key)->close();
    if(!silent)
    {
        emit sigChanged();
        emit sigModified();
    }
}

void CDiaryDB::delDiarys(const QStringList& keys)
{
    foreach(QString key,keys)
    {
        delDiary(key,true);
    }
    if(!keys.isEmpty())
    {
        emit sigChanged();
        emit sigModified();
    }
}

CDiary * CDiaryDB::getDiaryByKey(const QString& key)
{
    if(!diarys.contains(key)) return 0;

    return diarys[key];
}

void CDiaryDB::setModified(const QStringList& keys)
{
    foreach(QString key, keys)
    {
        if(diarys.contains(key))
        {
            diarys[key]->setModified();
        }
    }
}
