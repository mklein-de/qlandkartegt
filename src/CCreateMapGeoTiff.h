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
#ifndef CCREATEMAPGEOTIFF_H
#define CCREATEMAPGEOTIFF_H

#include <QWidget>
#include <QProcess>
#include <QPointer>
#include "ui_ICreateMapGeoTiff.h"

class QTemporaryFile;

class CCreateMapGeoTiff : public QWidget, private Ui::ICreateMapGeoTiff
{
    Q_OBJECT;
    public:
        CCreateMapGeoTiff(QWidget * parent);
        virtual ~CCreateMapGeoTiff();

        static CCreateMapGeoTiff * self(){return m_self;}

        struct refpt_t
        {
            refpt_t() : x(0), y(0), item(0){}

            double x;
            double y;

            QTreeWidgetItem * item;
        };

        enum columns_e
        {
              eNum          = 0
            , eLabel        = 1
            , eLonLat       = 2
            , eX            = 3
            , eY            = 4
            , eMaxColumn    = 5
        };

        QMap<quint32,refpt_t>& getRefPoints(){return refpts;}

    private slots:
        void slotOpenFile();
        void slotAddRef();
        void slotDelRef();
        void slotLoadRef();
        void slotSaveRef();
        void slotSelectionChanged();
        void slotItemChanged(QTreeWidgetItem * item, int column);
        void slotItemDoubleClicked(QTreeWidgetItem * item);
        void slotGoOn();
        void slotStderr();
        void slotStdout();
        void slotFinished( int exitCode, QProcess::ExitStatus status);
        void slotClearAll();

    private:
        static CCreateMapGeoTiff * m_self;

        void enableStep2();
        void enableStep3();

        void cleanupTmpFiles();

        QMap<quint32,refpt_t> refpts;
        quint32 refcnt;

        QProcess cmd;

        enum state_e {eNone, eTranslate, eWarp, eTile};
        state_e state;

        QPointer<QTemporaryFile> tmpfile1;
        QPointer<QTemporaryFile> tmpfile2;

        QString collect;
};
#endif                           //CCREATEMAPGEOTIFF_H
