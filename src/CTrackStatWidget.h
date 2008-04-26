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

#ifndef CTRACKSTATWIDGET_H
#define CTRACKSTATWIDGET_H

#include <QWidget>
#include <QPointer>

class CPlot;
class CTrack;

class CTrackStatWidget : public QWidget
{
    Q_OBJECT;
    public:
        CTrackStatWidget(QWidget * parent);
        virtual ~CTrackStatWidget();

    private slots:
        void slotChanged();

    private:
        CPlot * elevation;
        QPointer<CTrack> track;
};

#endif //CTRACKSTATWIDGET_H

