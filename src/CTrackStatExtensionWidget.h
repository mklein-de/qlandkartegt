/**********************************************************************************************
    Copyright (C) 2008 Oliver Eichler oliver.eichler@gmx.de
    Copyright (C) 2010 Christian Treffs ctreffs@gmail.com

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

#ifndef CTRACKSTATEXTENSIONWIDGET_H
#define CTRACKSTATEXTENSIONWIDGET_H

#include "ITrackStat.h"
#include "CTrackEditWidget.h"

class CTrackStatExtensionWidget : public ITrackStat
{
    Q_OBJECT;
    public:
        CTrackStatExtensionWidget(type_e type, QWidget * parent, QString name);
        virtual ~CTrackStatExtensionWidget();

    private slots:
        void slotChanged();
        void slotSetTrack(CTrack* track);

    private:
        QString myName;
        bool needResetZoom;
};
#endif                           //CTRACKSTATEXTENSIONWIDGET_H
