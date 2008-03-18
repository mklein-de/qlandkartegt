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
#ifndef CWPTTOOLWIDGET_H
#define CWPTTOOLWIDGET_H

#include <QWidget>
#include "ui_IWptToolWidget.h"

class QToolBox;
class QMenu;

/// waypoint tool view
class CWptToolWidget : public QWidget, private Ui::IWptToolWidget
{
    Q_OBJECT;
    public:
        CWptToolWidget(QTabWidget * parent);
        virtual ~CWptToolWidget();

        void selWptByKey(const QString& key);

    protected:
        void keyPressEvent(QKeyEvent * e);

    private slots:
        void slotDBChanged();
        void slotItemClicked(QListWidgetItem* item);
        void slotContextMenu(const QPoint& pos);
        void slotEdit();
        void slotDelete();
        void slotCopyPosition();

    private:
        QMenu * contextMenu;
};
#endif                           //CWPTTOOLWIDGET_H
