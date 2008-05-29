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
#ifndef CSEARCHTOOLWIDGET_H
#define CSEARCHTOOLWIDGET_H

#include <QWidget>
#include "ui_ISearchToolWidget.h"

class QToolBox;

/// search tool view
class CSearchToolWidget : public QWidget, private Ui::ISearchToolWidget
{
    Q_OBJECT;
    public:
        CSearchToolWidget(QTabWidget * parent);
        virtual ~CSearchToolWidget();

    private slots:
        void slotReturnPressed();
        void slotQueryFinished();
        void slotDBChanged();
        void slotItemClicked(QListWidgetItem* item);
        void slotContextMenu(const QPoint& pos);
        void slotDelete();
        void slotCopyPosition();
        void slotAdd();

    private:
        QMenu * contextMenu;

};
#endif                           //CSEARCHTOOLWIDGET_H
