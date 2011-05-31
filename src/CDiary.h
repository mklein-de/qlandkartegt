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

#ifndef CDIARY_H
#define CDIARY_H


#include "IItem.h"
#include <QDataStream>
#include <QFile>
#include <QPointer>

class CDiaryEditWidget;
class CTabWidget;

class CWpt;
class CTrack;
class CRoute;

struct db_diary_t
{
    ~db_diary_t()
    {
        qDeleteAll(wpts);
    }

    QString title;
    QList<CWpt*> wpts;
    QList<CTrack*> trks;
    QList<CRoute*> rtes;
};

class CDiary : public IItem
{
    Q_OBJECT;
    public:
        CDiary(QObject * parent);
        virtual ~CDiary();

        enum type_e {eEnd,eBase};

        QString getInfo();

        void setIcon(const QString&){}

        void linkToProject(quint64 key);

        void showEditWidget(CTabWidget * tab);

    private slots:
        void slotEditWidgetDied(QObject*);

    private:
        friend QDataStream& operator >>(QDataStream& s, CDiary& diary);
        friend QDataStream& operator <<(QDataStream& s, CDiary& diary);
        friend class CDiaryEditWidget;

        /// diary text
        QString m_text;

        quint64 keyProjectGeoDB;

        QPointer<CDiaryEditWidget> editWidget;

};

QDataStream& operator >>(QDataStream& s, CDiary& diary);
QDataStream& operator <<(QDataStream& s, CDiary& diary);

void operator >>(QFile& f, CDiary& diary);
void operator <<(QFile& f, CDiary& diary);
#endif                           //CDIARY_H
