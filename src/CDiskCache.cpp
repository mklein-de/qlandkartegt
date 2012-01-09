/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/  

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CDiskCache.cpp

  Module:      

  Description:

  Created:     01/09/2012

  (C) 2012 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "CDiskCache.h"
#include "CResources.h"

#include <QtGui>

CDiskCache::CDiskCache(QObject *parent)
: QObject(parent)
{

    dir     = CResources::self().getPathMapCache();
    maxSize = CResources::self().getSizeMapCache() * 1024*1024;

    if(!dir.exists("wms"))
    {
        dir.mkdir("wms");
    }
    dir.cd("wms");

    QFileInfoList files = dir.entryInfoList(QStringList("*.png"), QDir::Files);
    foreach(const QFileInfo& fileinfo, files)
    {
        QString hash    = fileinfo.baseName();
        table[hash]     = fileinfo.fileName();
    }

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
    table[hash]         = filename;

    img.save(dir.absoluteFilePath(filename));
}

void CDiskCache::restore(const QString& key, QImage& img)
{
    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(key.toAscii());

    QString hash = md5.result().toHex();

    if(table.contains(hash))
    {
        img.load(dir.absoluteFilePath(table[hash]));
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

    return table.contains(md5.result().toHex());

}

