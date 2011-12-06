/**********************************************************************************************
    Copyright (C) 2011 Oliver Eichler oliver.eichler@gmx.de

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
#ifndef CGRIDDB_H
#define CGRIDDB_H

#include <QObject>
#include <QColor>
#include <projects.h>

class QPainter;
class QRect;
class QCheckBox;

class CGridDB : public QObject
{
    Q_OBJECT;
    public:
        virtual ~CGridDB();

        static CGridDB& self(){return *m_pSelf;}

        void draw(QPainter& p, const QRect& rect, bool& needsRedraw);

        void setProjAndColor(const QString& proj, const QColor& color);

    private slots:
        void slotShowGrid(bool on){showGrid = on;}

    private:
        friend class CMainWindow;
        friend class CDlgSetupGrid;
        CGridDB(QObject * parent);
        void findGridSpace(double min, double max, double& xSpace, double& ySpace);
        bool calcIntersection(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4, double& x, double& y);

        static CGridDB * m_pSelf;

        PJ * pjWGS84;
        PJ * pjGrid;

        QCheckBox * checkGrid;
        bool showGrid;
        QString projstr;
        QColor color;
};

#endif //CGRIDDB_H

