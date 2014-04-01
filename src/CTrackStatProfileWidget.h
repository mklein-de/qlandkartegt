/**********************************************************************************************
    Copyright (C) 2008 Oliver Eichler oliver.eichler@gmx.de

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

**********************************************************************************************/

#ifndef CTRACKSTATPROFILEWIDGET_H
#define CTRACKSTATPROFILEWIDGET_H

#include "ITrackStat.h"

class CTrackStatProfileWidget : public ITrackStat
{
    Q_OBJECT;
    public:
        CTrackStatProfileWidget(type_e type, QWidget * paren);
        virtual ~CTrackStatProfileWidget();
    private:
        bool needResetZoom;
    private slots:
        void slotChanged();
        void slotSetTrack(CTrack *track);

};
#endif                           //CTRACKSTATPROFILEWIDGET_H
