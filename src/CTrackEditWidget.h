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
#ifndef CTRACKEDITWIDGET_H
#define CTRACKEDITWIDGET_H

#include <QWidget>
#include <QObject>
#include <QBoxLayout>
#include <QPointer>
#include <CGpxExtension.h>
#include <QVBoxLayout>
#include "ui_ITrackEditWidget.h"

class CTrack;
class CTrackStatProfileWidget;
class CTrackStatSpeedWidget;
class CTrackStatTraineeWidget;

//TODO: Class Defininition
class CTrackStatExtensionWidget;

class CTrackTreeWidgetItem : public QTreeWidgetItem
{

    public:
        CTrackTreeWidgetItem(QTreeWidget *tree) : QTreeWidgetItem(tree)
        {
        }

        CTrackTreeWidgetItem ( QTreeWidget * parent, const QStringList & strings) : QTreeWidgetItem (parent,strings)
        {
        }

        bool operator< ( const QTreeWidgetItem & other ) const;
};

class CTrackEditWidget : public QWidget, private Ui::ITrackEditWidget
{
    Q_OBJECT;
    public:
        CTrackEditWidget(QWidget * parent);
        virtual ~CTrackEditWidget();

    public slots:
        void slotSetTrack(CTrack * t);
        void slotCheckReset(bool checked);
        void slotCheckRemove(bool checked);
        void slotApply();
        void slotPointSelectionChanged();
        void slotPointSelection(QTreeWidgetItem * item);
        void slotPurge();
        void slotUpdate();
        void slotToggleStatDistance();
        void slotToggleStatTime();
        void slotToggleTrainee();

        //TODO: Deklaration der Methode fr die Extensions Graphen
        void slotToggleExtensionsGraph();

        //TODO: eigenes Methdchen
        void slotSetColumns(bool checked);
        void slotSetColumnsExt(bool checked);

        void slotGoogleMaps();   //TODO: Google Maps
                                 //TODO: Kill Tab
        void slotKillTab(int index);

    protected:
        void keyPressEvent(QKeyEvent * e);

    private:
        enum columns_e
        {
            eNum       = 0
            ,eTime      = 1
            ,eAltitude  = 2
            ,eDelta     = 3
            ,eAzimuth   = 4
            ,eDistance  = 5
            ,eSpeed     = 6
            ,eAscend    = 7
            ,eDescend   = 8
            ,ePosition  = 9
            ,eMaxColumn = 10
        };

        QPointer<CTrack> track;

        bool originator;

        QPointer<CTrackStatProfileWidget> trackStatProfileDist;
        QPointer<CTrackStatSpeedWidget> trackStatSpeedDist;
        QPointer<CTrackStatProfileWidget> trackStatProfileTime;
        QPointer<CTrackStatSpeedWidget> trackStatSpeedTime;
        QPointer<CTrackStatTraineeWidget> trackStatTrainee;

                                 //TODO: Namen der extensions
        QList<QString> names_of_ext;
        int num_of_ext;          //TODO: Anzahl der extensions

        QList<QCheckBox *> c_boxes;

                                 //TODO: QPointer for tabs to be handled
        QPointer<CTrackStatExtensionWidget> tab;

                                 //TODO: QList with all extension tabs made for further handling
        QList<CTrackStatExtensionWidget *> tabs;

        QSpacerItem *Vspace;     //TODO: Spacer Item

        QLabel *label;

        int tabstat;
        int no_ext_info_stat;
        int count;

};
#endif                           //CTRACKEDITWIDGET_H
