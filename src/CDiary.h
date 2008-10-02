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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/

#ifndef CDIARY_H
#define CDIARY_H

#include <QObject>
#include <QDataStream>
#include <QFile>

class CDiary : public QObject
{
    Q_OBJECT;
    public:
        CDiary(QObject * parent);
        CDiary(const CDiary& parent);
        virtual ~CDiary();

        enum type_e {eEnd,eBase};

        QString text(){return m_text;}
        void setText(const QString& t){m_text = t;}

        CDiary& operator=(const CDiary& d) {
            setParent(d.parent());
            timestamp   = d.timestamp;
            m_text      = d.m_text;
            return *this;
        }

    private:
        friend QDataStream& operator >>(QDataStream& s, CDiary& diary);
        friend QDataStream& operator <<(QDataStream& s, CDiary& diary);

        /// creation timestamp
        quint32 timestamp;
        /// diary text
        QString m_text;

};

QDataStream& operator >>(QDataStream& s, CDiary& diary);
QDataStream& operator <<(QDataStream& s, CDiary& diary);

void operator >>(QFile& f, CDiary& diary);
void operator <<(QFile& f, CDiary& diary);
#endif                           //CDIARY_H
