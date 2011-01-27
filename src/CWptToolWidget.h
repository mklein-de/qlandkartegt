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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/
#ifndef CWPTTOOLWIDGET_H
#define CWPTTOOLWIDGET_H

#include <QWidget>
#include "ui_IWptToolWidget.h"

class QToolBox;
class QMenu;
class QAction;

/// waypoint tool view
class CWptToolWidget : public QWidget, private Ui::IWptToolWidget
{
    Q_OBJECT;
    public:
        CWptToolWidget(QTabWidget * parent);
        virtual ~CWptToolWidget();
        void selWptByKey(const QString& key);

        enum sortmode_e
        {
             eSortByName
            ,eSortByComment
            ,eSortByIcon
            ,eSortByDistance
        };

        static sortmode_e  getSortMode(QString& pos){pos = sortpos; return sortmode;}

    signals:
        void sigChanged();

    protected:
        void keyPressEvent(QKeyEvent * e);

    private slots:
        void slotDBChanged();
        void slotItemClicked(QListWidgetItem* item);
        void slotContextMenu(const QPoint& pos);
        void slotEdit();
        void slotDelete();
        void slotDeleteBy();
        void slotCopyPosition();
        void slotProximity();
        void slotMakeRoute();
        void slotZoomToFit();
        void slotPosTextChanged(const QString& text);
        void slotShowNames();

    private:
        QMenu * contextMenu;
        QAction * actCopyPos;
        QAction * actEdit;
        QAction * actProximity;
        QAction * actMakeRte;
        QAction * actZoomToFit;
        QAction * actDelete;
        QAction * actDeleteBy;
        QAction * actShowNames;

        static sortmode_e sortmode;
        static QString sortpos;

};
#endif                           //CWPTTOOLWIDGET_H
