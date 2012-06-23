/**********************************************************************************************
    Copyright (C) 2012 Oliver Eichler oliver.eichler@gmx.de

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

#ifndef CTRACKFILTERWIDGET_H
#define CTRACKFILTERWIDGET_H

#include <QWidget>
#include <QPointer>

#include "ui_ITrackFilterWidget.h"

class CTrackEditWidget;
class CTrack;

class CTrackFilterWidget : public QWidget, private Ui::ITrackFilterWidget
{
    Q_OBJECT;
    public:
        CTrackFilterWidget(QWidget * parent);
        virtual ~CTrackFilterWidget();

        void setTrackEditWidget(CTrackEditWidget * w);

    private slots:
        void slotApplyFilter();
        void slotHighlightTrack(CTrack * trk);
        void slotComboMeterFeet(const QString &text);
        void slotResetFilterList();
        void slotAddFilterHidePoints1();
        void slotAddFilterSmoothProfile1();
        void slotAddFilterSplit1();
        void slotAddFilterSplit2();
        void slotAddFilterSplit3();
        void slotAddFilterSplit4();

    private:
        void addFilter(const QString& name, const QString& icon, QByteArray& args);
        bool filterHidePoints1(QDataStream &args, QList<CTrack *> &tracks);
        bool filterSmoothProfile1(QDataStream &args, QList<CTrack *> &tracks);
        bool filterSplit1Tracks(QDataStream &args, QList<CTrack *> &tracks);
        bool filterSplit1Stages(QDataStream &args, QList<CTrack *> &tracks);
        bool filterSplit2Tracks(QDataStream &args, QList<CTrack *> &tracks);
        bool filterSplit2Stages(QDataStream &args, QList<CTrack *> &tracks);
        bool filterSplit3Tracks(QDataStream &args, QList<CTrack *> &tracks);
        bool filterSplit3Stages(QDataStream &args, QList<CTrack *> &tracks);
        bool filterSplit4Tracks(QDataStream &args, QList<CTrack *> &tracks);
        bool filterSplit4Stages(QDataStream &args, QList<CTrack *> &tracks);


        enum filterType_e {eHidePoints1, eSmoothProfile1, eSplit1, eSplit2, eSplit3, eSplit4};

        QPointer<CTrackEditWidget> trackEditWidget;
        QPointer<CTrack> track;
};

#endif //CTRACKFILTERWIDGET_H

