/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de

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
#ifndef CDLGCREATEMAP_H
#define CDLGCREATEMAP_H

#include <QDialog>

#include "ui_IDlgCreateMap.h"

class CCreateMapOSM;
class CCreateMapQMAP;

/// dialog to hold several map creation dialogs
class CDlgCreateMap : public QDialog, private Ui::IDlgCreateMap
{
    Q_OBJECT
    public:
        CDlgCreateMap(QWidget * parent);
        virtual ~CDlgCreateMap();

        void editMap(const QString& filename);

    private:
        enum widget_e {eNone, eOSM, eQMAP};


        CCreateMapOSM  * widgetOSM;
        CCreateMapQMAP * widgetQMAP;
};

#endif //CDLGCREATEMAP_H

