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
#ifndef CTRACKDB_H
#define CTRACKDB_H

#include "IDB.h"
#include <QRectF>
#include <QMap>
class QToolBox;
class CTrack;
class CTrackToolWidget;

class CTrackDB : public IDB
{
    Q_OBJECT;
    public:
        virtual ~CTrackDB();

        static CTrackDB& self(){return *m_self;}

        void loadGPX(CGpx& gpx);
        void saveGPX(CGpx& gpx);
        void loadQLB(CQlb& qlb);
        void saveQLB(CQlb& qlb);

        void upload();
        void download();

        void clear();

        void addTrack(CTrack* track, bool silent);
        void delTrack(const QString& key, bool silent);
        void delTracks(const QStringList& keys);
        void CombineTracks();
        void splitTrack(int idx);

        void highlightTrack(const QString& key);
        /// get highlighted track
        /**
            <b>WARNING</b> The object referenced by the returned
            pointer might be subject to destruction at any time.
            Thus you must use it temporarily or store it by a
            QPointer object.

            @return A pointer to the current highlighted track or 0.
        */
        CTrack* highlightedTrack();

        /// get access to track dictionary
        const QMap<QString,CTrack*>& getTracks(){return tracks;}

        CTrackToolWidget * getToolWidget();

        int count(){return tracks.count();}

        QRectF getBoundingRectF(const QString key);
        QRectF getBoundingRectF();

        void draw(QPainter& p, const QRect& rect, bool& needsRedraw);

        void select(const QRect& rect);

        signals:
        void sigHighlightTrack(CTrack * track);

    private:
        friend class CMainWindow;
        friend class CTrackEditWidget;

        CTrackDB(QTabWidget * tb, QObject * parent);
        quint32 cnt;

        static CTrackDB * m_self;

        QMap<QString,CTrack*> tracks;
};
#endif                           //CTRACKDB_H
