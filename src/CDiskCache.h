/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/  

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CDiskCache.h

  Module:      

  Description:

  Created:     01/09/2012

  (C) 2012 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CDISKCACHE_H
#define CDISKCACHE_H

#include <QObject>
#include <QDir>
#include <QHash>

class QImage;

class CDiskCache : public QObject
{
    Q_OBJECT;
    public:
        CDiskCache(QObject * parent);
        virtual ~CDiskCache();

        void store(const QString& key, QImage& img);
        void restore(const QString& key, QImage& img);
        bool contains(const QString& key);

    private:
        QDir dir;
        quint32 maxSize;

        QHash<QString, QString> table;
};

#endif //CDISKCACHE_H

