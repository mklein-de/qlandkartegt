/**********************************************************************************************
    Copyright (C) 2012 Oliver Eichler oliver.eichler@gmx.de

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

**********************************************************************************************/

#ifndef CIMAGESELECT_H
#define CIMAGESELECT_H

#include <QWidget>
#include "ui_IImageSelect.h"

class CWpt;

class CImageSelect : public QWidget, private Ui::IImageSelect
{
    Q_OBJECT;
    public:
        CImageSelect(QWidget * parent);
        virtual ~CImageSelect();

        void setWpt(CWpt * pt){wpt = pt;}

        void setTransparent(bool yes);

        struct img_t
        {
            img_t(const QString& title, const QString& fn, const QString& src)
                : img(src)
                , title(title)
                , filename(fn)
            {

            }

            QPixmap img;
            QString title;
            QString filename;
        };

        signals:
        void sigSelectImage(const CImageSelect::img_t& img);

    protected:
        void paintEvent(QPaintEvent * e);
        void resizeEvent(QResizeEvent * e);
        void mousePressEvent(QMouseEvent * e);
        void wheelEvent(QWheelEvent * e);

        QList<img_t> images;
        CWpt * wpt;
};
#endif                           //CIMAGESELECT_H
