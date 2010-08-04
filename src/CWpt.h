/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de

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
#ifndef CWPT_H
#define CWPT_H

#include <QObject>
#include <QPixmap>
#include <QString>
#include <QList>
#include <QFile>
#include <QDir>

#define WPT_NOFLOAT 1e25f

class CWptDB;

/// waypoint data object
/**
    The idea of this waypoint data onject is to provide an appendable data format, that
    will fit all needs of the future. For the sake of backward compatibility <b>never append
    an existing functional data block</b>. Define a new one. <b>And never assume a block other
    than base to exist.</b> Use the type values from the offset table and read those functional
    data block you understand.

    Data structure for serialized waypoint objects:

<pre>
    "QLWpt   "\0                        // 9 bytes magic string
    qint32 type, quint32 offset         // offset table for functional data blocks
    ...        , ...                    // the type is one of CWpt:type_e, the offset
    ...        , ...                    // is the number of bytes right from the beginning
    0          , void                   // The table is terminated by eEnd (0)
    QByteArray 1                        // Each functional data block is stored in it's own
    ...                                 // byte array. You can randomly access these blocks
    QByteArray N                        // by seeking the offset and stream the data into
                                        // a QByteArray object.

    functional data block eBase:
    QString _key_;                      // unique key to identify the waypoint
    quint32 sticky;                     // the sticky flag (always keep waypoint)
    quint32 timestamp;                  // the creation timestamp of the waypoint
    QString icon;                       // the icon type string
    QString name;                       // waypoint name
    QString comment;                    // waypoint comment (HTML)
    float   lat;                        // latitude []
    float   lon;                        // longitude []
    float   altitude;                   // well, the altitude [m]
    float   proximity;                  // a radius for proximity alerts [m]

    functional data block eImage:
    quint32 offset 1                    // for each image an offset into the functional data block
    ...                                 // is stored.
    quint32 offset N                    //
    quint32 0                           // the offset table is terminated by a value of 0
    QString info1, QPixmap image1       // each image is stored as QPixmap (some kind of png format)
    ...          , ...                  // and an informational string.
    QString infoN, QPixmap imageN

</pre>
*/
class CWpt : public QObject
{
    Q_OBJECT;
    public:
        CWpt(CWptDB * parent);
        virtual ~CWpt();

        const QString& key();
        const QString filename(const QDir& dir = CWpt::path);
        enum type_e {eEnd,eBase,eImage};
        static QDir& getWptPath(){return path;}
        static void resetKeyCnt(){keycnt = 0;}


    private:
        void genKey();

        // eBase: base information
    private:
        friend QDataStream& operator >>(QDataStream& s, CWpt& wpt);
        friend QDataStream& operator <<(QDataStream& s, CWpt& wpt);
        friend class CDlgEditWpt;
        friend class CDlgWptEdit;
        static QDir path;
        static quint32 keycnt;
        QString _key_;

    public:
        quint32 sticky;
        quint32 timestamp;
        QString icon;
        QString name;
        QString comment;
        float   lat;             ///< [deg]
        float   lon;             ///< [deg]
        float   ele;             ///< [m]
        float   prx;             ///< [m]
        QString link;

        struct image_t
        {
            quint32 offset;
            QString info;
            QPixmap pixmap;
            QString filePath;
        };
        QList<image_t> images;

};

QDataStream& operator >>(QDataStream& s, CWpt& wpt);
QDataStream& operator <<(QDataStream& s, CWpt& wpt);

void operator >>(QFile& s, CWpt& wpt);
void operator <<(QFile& s, CWpt& wpt);
#endif                           //CWPT_H
