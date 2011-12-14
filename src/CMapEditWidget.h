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
#ifndef CMAPEDITWIDGET_H
#define CMAPEDITWIDGET_H

#include <QWidget>

#include "ui_IMapEditWidget.h"

class CCreateMapOSM;
class CCreateMapQMAP;
class CCreateMapGeoTiff;
class CCreateMapFineTune;
class CCreateMapWMS;

class CMapEditWidget : public QWidget, private Ui::IMapEditWidget
{
    Q_OBJECT;
    public:
        CMapEditWidget(QWidget * parent);
        virtual ~CMapEditWidget();

    private:
        enum widget_e {eNone, eOSM, eQMAP, eGTIFF, eFineTune, eWMS};

        CCreateMapOSM  * widgetOSM;
        CCreateMapQMAP * widgetQMAP;
        CCreateMapGeoTiff * widgetGeoTiff;
        CCreateMapFineTune * widgetFineTune;
};
#endif                           //CMAPEDITWIDGET_H
