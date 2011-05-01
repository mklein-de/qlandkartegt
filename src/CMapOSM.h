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

#ifndef CMAPOSM_H
#define CMAPOSM_H

#include <IMap.h>
#include "CMapOSMType.h"

class COsmTilesHash;
class CCanvas;
class QComboBox;
#include <QPair>
class CMapOSM : public IMap
{
    Q_OBJECT;
    public:
        CMapOSM(CCanvas * parent);
        virtual ~CMapOSM();

        void convertPt2M(double&, double&);
        void convertM2Pt(double&, double&);
        void move(const QPoint&, const QPoint&);
        void zoom(bool, const QPoint&);
        void zoom(double lon1, double lat1, double lon2, double lat2);
        void zoom(qint32& level);
        void select(const QRect&){};
        void dimensions(double& lon1, double& lat1, double& lon2, double& lat2);
        void getArea_n_Scaling(XY& p1, XY& p2, float& my_xscale, float& my_yscale);
        void draw(QPainter& p);

        QList<CMapOSMType> getServerList(){return tileList;}
        bool rebuildServerList();

    public slots:
        void newImageReady(QImage image, bool lastTileLoaded);
        void setNewTileUrl(int cbIndex = -1);
    private:
        QComboBox *cb;
        QWidget *parent;
        int currentTileListIndex;
        QList<CMapOSMType> tileList;
        QImage image;
        bool lastTileLoaded;
        void draw();
        COsmTilesHash *osmTiles;
        double zoomFactor;
        void config();
        ///actual x offset in [m]
        double x;
        ///actual y offset in [m]
        double y;

        /// scale [px/m]
        double xscale;
        /// scale [px/m]
        double yscale;

        /// reference point [m] (left hand side of map)
        double xref1;
        /// reference point [m] (top of map)
        double yref1;
        /// reference point [m] (right hand side of map)
        double xref2;
        /// reference point [m] (bottom of map)
        double yref2;

        /// the longitude of the top left reference point [rad]
        double lon1;
        /// the latitude of the top left reference point [rad]
        double lat1;
        /// the longitude of the bottom right reference point [rad]
        double lon2;
        /// the latitude of the bottom right reference point [rad]
        double lat2;

        bool needsRedrawOvl;

        // builtin maps
        CMapOSMType mapOsm;
        CMapOSMType mapOcm;
        CMapOSMType mapOpm;
        CMapOSMType mapWam;
#if PRIVATE
        CMapOSMType mapOade;
        CMapOSMType mapOaat;
        CMapOSMType mapOait;
#endif
};
#endif                           //CMAPOSM_H
