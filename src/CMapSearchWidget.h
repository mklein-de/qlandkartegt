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
#ifndef CMAPSEARCHWIDGET_H
#define CMAPSEARCHWIDGET_H

#include "CMapSelectionRaster.h"
#include <QWidget>
#include <QPointer>
#include "ui_IMapSearchWidget.h"

class CMapSearchCanvas;
class QPixmap;
class CImage;
class CMapSearchThread;
class QTimer;
class QListWidgetItem;

class CMapSearchWidget : public QWidget, private Ui::IMapSearchWidget
{
    Q_OBJECT;
    public:
        CMapSearchWidget(QWidget * parent);
        virtual ~CMapSearchWidget();

        void setArea(CMapSelectionRaster& ms);

    private slots:
        void slotSelectArea();
        void slotSelectMask();
        void slotSelectMaskByName(const QString& name);
        void slotSearch();
        void slotThreshold(int i);
        void slotMaskSelection(const QPixmap& pixmap);
        void slotDeleteMask();
        void slotSaveMask();
        void slotSearchFinished();
        void slotProgressSymbol(const QString& status, const int progress);
        void slotCancel();
        void slotMapChanged();

#ifdef SQL_SEARCH_GARMIN
        void slotProgressIndex(const QString& status, const int progress);
        void slotCreateIndex();
        void slotIndexChanged();
        void slotLineSearchChanged();
        void slotLineSearchChanged(const QString&);
        void slotLineSelected();

        void slotPointSearchChanged();
        void slotPointSearchChanged(const QString&);
        void slotPointSelected();
#endif                       //SQL_SEARCH_GARMIN

    private:
        void binarizeViewport(int t);
        void loadMaskCollection();
        void checkGui();

        CMapSelectionRaster area;
        QPointer<CMapSearchCanvas> canvas;
        CImage * mask;

        CMapSearchThread * thread;

#ifdef SQL_SEARCH_GARMIN
        QTimer * triggerLineSearch;
        QTimer * triggerPointSearch;
#endif
};
#endif                           //CMAPSEARCHWIDGET_H
