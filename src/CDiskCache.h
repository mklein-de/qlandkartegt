/**********************************************************************************************
    Copyright (C) 2012 Oliver Eichler oliver.eichler@gmx.de

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

#ifndef CDISKCACHE_H
#define CDISKCACHE_H

#include <QObject>
#include <QDir>
#include <QHash>
#include <QImage>

class QTimer;

class CDiskCache : public QObject
{
    Q_OBJECT;
    public:
#ifdef STANDALONE
        CDiskCache(const QString &path, QObject *parent);
#else
        CDiskCache(QObject *parent);
#endif //STANDALONE
        virtual ~CDiskCache();

        void store(const QString& key, QImage& img);
        void restore(const QString& key, QImage& img);
        bool contains(const QString& key);

    private slots:
        void slotCleanup();
    private:
        QDir dir;

        /// hash table to cache images als files on disc
        QHash<QString, QString> table;
        /// hash table to cache loaded images in memory
        QHash<QString, QImage>  cache;

        QTimer * timer;

        QImage dummy;
};

#endif //CDISKCACHE_H

