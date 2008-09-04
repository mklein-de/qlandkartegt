//C-  -*- C++ -*-
//C- -------------------------------------------------------------------
//C- Copyright (c) 2008 Marc Feld
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, either version 2 of the license,
//C- or (at your option) any later version. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C-
//C- This program is distributed in the hope that it will be useful,
//C- but WITHOUT ANY WARRANTY; without even the implied warranty of
//C- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//C- GNU General Public License for more details.
//C-  ------------------------------------------------------------------

#ifndef TCXREADER_H_
#define TCXREADER_H_
#include <QXmlStreamReader>
#include <QString>
#include "CTrack.h"
class QObject;
class CTrack;

class TcxReader : public QXmlStreamReader
{
public:
    TcxReader(QObject *parent);
    virtual ~TcxReader();
    bool read(QString path);
    bool read(QIODevice *device);
private:
    QObject *parent;
    CTrack::pt_t pold;
    bool firstPositionFound;
    void readUnknownElement();
    void readTcx();
    void readActivities();
    void readActivity();
    void readLap(CTrack *track);
    void readTrack(CTrack *track, int lap);
    void readTrackpoint(CTrack *track, int lap);
    void readHeartRateBpm(CTrack::pt_t *pt);
    void readPosition(CTrack::pt_t *pt);

};

#endif /* TCXREADER_H_ */
