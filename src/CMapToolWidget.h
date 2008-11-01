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
#ifndef CMAPTOOLWIDGET_H
#define CMAPTOOLWIDGET_H

#include <QWidget>
#include "ui_IMapToolWidget.h"

class QToolBox;
class QPoint;
class QListWidgetItem;

class CMapToolWidget : public QWidget, private Ui::IMapToolWidget
{
    Q_OBJECT;
    public:
        CMapToolWidget(QTabWidget * parent);
        virtual ~CMapToolWidget();

    private slots:
        void slotDBChanged();
        void slotKnownMapClicked(QTreeWidgetItem* item, int);
        void slotSelectedMapClicked(QListWidgetItem* item);
        void slotSelectMap(QListWidgetItem* item);
        void slotContextMenuKnownMaps(const QPoint& pos);
        void slotContextMenuSelectedMaps(const QPoint& pos);
        void slotDeleteKnownMap();
        void slotDeleteSelectedMap();
        void slotExportMap();

    private:
        enum columns_e
        {
             eMode
            ,eType
            ,eName
            ,eMaxColumn
        };
        void updateEportButton();
        QMenu * contextMenuKnownMaps;
        QMenu * contextMenuSelectedMaps;

};
#endif                           //CMAPTOOLWIDGET_H
