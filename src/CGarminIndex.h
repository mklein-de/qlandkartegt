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

#ifndef CGARMININDEX_H
#define CGARMININDEX_H

#include <QThread>
#include <QStringList>
#include <QSqlDatabase>
#include <QMutex>

class CGarminIndex : public QThread
{
    Q_OBJECT;
    public:
        CGarminIndex(QObject * parent);
        virtual ~CGarminIndex();
        void open(const QString& name);
        void create(const QStringList& files);

    signals:
        void sigProgress(const QString& status, const int progress);

    protected:
        void run();

    private:
        QMutex          mutex;
        QString         dbName;
        QStringList     imgFiles;
        QSqlDatabase    db;
};

#endif //CGARMININDEX_H

