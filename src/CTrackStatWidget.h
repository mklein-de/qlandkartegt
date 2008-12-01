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

#ifndef CTRACKSTATWIDGET_H
#define CTRACKSTATWIDGET_H

#include <QWidget>
#include <QPointer>

#include "ui_ITrackStatWidget.h"

class CPlot;
class CTrack;

class CTrackStatWidget : public QWidget, protected Ui::ITrackStatWidget
{
    Q_OBJECT;
    public:
        CTrackStatWidget(QWidget * parent);
        virtual ~CTrackStatWidget();

    protected:
        void mousePressEvent(QMouseEvent * e);

    private slots:
        void slotChanged();

    private:
        CPlot * elevation;
        CPlot * speed;
        QPointer<CTrack> track;
};
#endif                           //CTRACKSTATWIDGET_H
