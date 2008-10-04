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

#ifndef CIMAGE_H
#define CIMAGE_H

#include <QObject>
#include <QImage>
#include <QPixmap>

class CImage : public QObject
{
    Q_OBJECT;
    public:
        CImage(const QPixmap& pix, QObject * parent = 0);
        virtual ~CImage();

        int getThreshold(){return threshold;}
        const QImage& binarize(int threshold);
        QImage mask();

    private:
        int calcThreshold(const QVector<double>& hist);

        QVector<QRgb> graytable;
        QVector<QRgb> bintable;
        QImage  imgRgb;
        QImage  imgGray;
        QImage  imgBinary;

        QVector<double> grayHistogram;

        int threshold;
};

#endif //CIMAGE_H

