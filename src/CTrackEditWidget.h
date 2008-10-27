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
#ifndef CTRACKEDITWIDGET_H
#define CTRACKEDITWIDGET_H

#include <QWidget>
#include <QPointer>
#include "ui_ITrackEditWidget.h"

class CTrack;
class CTrackStatProfileWidget;
class CTrackStatSpeedWidget;
class CTrackStatTraineeWidget;
#ifdef PLOT_3D
class CMap3DWidget;
#endif

class CTrackEditWidget : public QWidget, private Ui::ITrackEditWidget
{
    Q_OBJECT;
    public:
        CTrackEditWidget(QWidget * parent);
        virtual ~CTrackEditWidget();

    public slots:
        void slotSetTrack(CTrack * t);
        void slotCheckReset(bool checked);
        void slotCheckRemove(bool checked);
        void slotApply();
        void slotPointSelectionChanged();
        void slotPointSelection(QTreeWidgetItem * item);
        void slotPurge();
        void slotUpdate();
        void slotToggleStat();
        void slotToggleTrainee();

    protected:
        void keyPressEvent(QKeyEvent * e);

    private:
        enum columns_e
        {
            eNum       = 0
            ,eTime      = 1
            ,eAltitude  = 2
            ,eDelta     = 3
            ,eAzimuth   = 4
            ,eDistance  = 5
            ,eSpeed     = 6
            ,eAscend    = 7
            ,eDescend   = 8
            ,ePosition  = 9
            ,eMaxColumn = 10
        };

        QPointer<CTrack> track;

        bool originator;

        QPointer<CTrackStatProfileWidget> trackStatProfile;
        QPointer<CTrackStatSpeedWidget> trackStatSpeed;
        QPointer<CTrackStatTraineeWidget> trackStatTrainee;
#ifdef PLOT_3D
        QPointer<CMap3DWidget> track3D;
#endif

};
#endif                           //CTRACKEDITWIDGET_H
