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

#include "CDiskCache.h"
#ifndef STANDALONE
#include "CResources.h"
#endif  //!STANDALONE

#include <QtGui>

#ifdef STANDALONE
CDiskCache::CDiskCache(const QString &path, QObject *parent)
#else
CDiskCache::CDiskCache(QObject *parent)
#endif //STANDALONE
: QObject(parent)
, dummy(":/icons/noMap256x256.png")
{

#ifdef STANDALONE
    dir     = QDir(path);
    maxSize = -1;
#else
    dir     = CResources::self().getPathMapCache();
    maxSize = CResources::self().getSizeMapCache() * 1024*1024;
#endif //STANDALONE

    QFileInfoList files = dir.entryInfoList(QStringList("*.png"), QDir::Files);
    foreach(const QFileInfo& fileinfo, files)
    {
        QString hash    = fileinfo.baseName();
        table[hash]     = fileinfo.fileName();
    }

#ifndef STANDALONE
    timer = new QTimer(this);
    timer->setSingleShot(false);
    timer->start(60000);
    connect(timer, SIGNAL(timeout()), this, SLOT(slotCleanup()));
    slotCleanup();
#endif // !STANDALONE

    //qDebug() << "cache: found" << table.count() << "cache entries";
}



CDiskCache::~CDiskCache()
{

}

void CDiskCache::store(const QString& key, QImage& img)
{
    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(key.toAscii());

    QString hash        = md5.result().toHex();
    QString filename    = QString("%1.png").arg(hash);



    if(!img.isNull())
    {
        img.save(dir.absoluteFilePath(filename));
        table[hash] = filename;
        cache[hash] = img;
    }
    else
    {
        cache[hash] = dummy;
    }
}

void CDiskCache::restore(const QString& key, QImage& img)
{
    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(key.toAscii());

    QString hash = md5.result().toHex();

    if(cache.contains(hash))
    {
        img = cache[hash];
    }
    else if(table.contains(hash))
    {
        img.load(dir.absoluteFilePath(table[hash]));
        if(!cache.contains(hash))
        {
            cache[hash] = img;
        }
    }
    else
    {
        img = QImage();
    }

}

bool CDiskCache::contains(const QString& key)
{
    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(key.toAscii());

    QString hash = md5.result().toHex();
    return table.contains(hash) || cache.contains(hash);

}

void CDiskCache::slotCleanup()
{
    qint64 size = 0;
    QFileInfoList files = dir.entryInfoList(QStringList("*.png"), QDir::Files, QDir::Time|QDir::Reversed);
    QDateTime now = QDateTime::currentDateTime();

    // expire old files and calculate cache size
    foreach(const QFileInfo& fileinfo, files)
    {
        if(fileinfo.lastModified().daysTo(now) > 8)
        {
            QString hash = fileinfo.baseName();
            table.remove(hash);
            cache.remove(hash);
            QFile::remove(fileinfo.absoluteFilePath());
        }
        else
        {
            size += fileinfo.size();
        }
    }


    if(size > maxSize)
    {
        // if cache is still too large remove oldest files
        foreach(const QFileInfo& fileinfo, files)
        {
            QString hash = fileinfo.baseName();
            table.remove(hash);
            cache.remove(hash);
            QFile::remove(fileinfo.absoluteFilePath());

            size -= fileinfo.size();

            if(size < maxSize)
            {
                break;
            }
        }
    }   
}