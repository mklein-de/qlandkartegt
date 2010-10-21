/**********************************************************************************************
    Copyright (C) 2009 Oliver Eichler oliver.eichler@gmx.de

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
#ifndef CROUTETOOLWIDGET_H
#define CROUTETOOLWIDGET_H

#include <QWidget>
#include "ui_IRouteToolWidget.h"

class CRoute;

class CRouteToolWidget : public QWidget, private Ui::IRouteToolWidget
{
    Q_OBJECT;
    public:
        CRouteToolWidget(QTabWidget * parent);
        virtual ~CRouteToolWidget();

    protected:
        void keyPressEvent(QKeyEvent * e);

    private slots:
        void slotDBChanged();
        void slotItemClicked(QListWidgetItem * item);
        void slotItemDoubleClicked(QListWidgetItem * item);
        void slotContextMenu(const QPoint& pos);
        void slotEdit();
        void slotDelete();
        void slotCalcRoute();

    private:
        void startOpenRouteService(CRoute& rte);

        bool originator;

        enum tab_e {
            eTabRoute = 0
            ,eTabSetup = 1
        };

        enum service_e
        {
            eOpenRouteService
        };

        static const QString gml_ns;
        static const QString xls_ns;
        static const QString xsi_ns;
        static const QString sch_ns;
        static const QString xlink_ns;
        static const QString schemaLocation;

};
#endif                           //CROUTETOOLWIDGET_H
