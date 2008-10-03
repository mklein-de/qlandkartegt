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

#include "CImage.h"

#include <QtGui>

CImage::CImage(const QPixmap& pix, QObject * parent)
: QObject(parent)
, grayHistogram(256, 0.0)
, threshold(128)
{
    QRect rect  = pix.rect();
    rect.setWidth((rect.width() >> 2) << 2);
    imgRgb      = pix.copy(rect).toImage();

    int i;
    for(i = 0; i < 256; ++i) {
        graytable << qRgb(i,i,i);
    }
    imgGray = QImage(rect.size(), QImage::Format_Indexed8);
    imgGray.setColorTable(graytable);

    QRgb *   p1 = (QRgb*)imgRgb.bits();
    quint8 * p2 = (quint8*)imgGray.bits();

    const int nPixel = imgGray.width() * imgGray.height();
    for(i = 0; i < nPixel; ++i, ++p1, ++p2){
        *p2 = qGray(*p1);
        grayHistogram[*p2] += 1.0;
    }

    double Sum = 0;
    for(int j = 0 ; j < 256 ; j++ ) {
        Sum += grayHistogram[j] ;
    }

    for(int j = 0 ; j < 256 ; j++ ) {
        grayHistogram[j] /= Sum ;
    }

    threshold = calcThreshold(grayHistogram);
}

CImage::~CImage()
{

}

int CImage::calcThreshold(const QVector<double>& hist)
{
    // histogram histo;
    int nThreshold;
    double criterion;
    double expr_1;

    // double p[256];
    double omega_k;
    double sigma_b_k;
    double sigma_T;
    double fMu_T;
    double fMu_k;
    int k_low, k_high;
    double fMu_0;
    double fMu_1;
    double fMu;

    fMu_T   = 0.0;
    for (int i = 0 ; i < 256 ; i++) {
        fMu_T += i * hist[i];
    }

    //Standard deviation
    sigma_T = 0.0;
    for (int i = 0; i < 256 ; i++) {
        sigma_T += (i - fMu_T ) * (i - fMu_T ) * hist[i];
    }

    //Means for classes c1 and c2
    //c'est un peu lger!!!!
    for (k_low = 0;   (hist[k_low]  == 0) && (k_low  < 255); k_low++);
    for (k_high =255; (hist[k_high] == 0) && (k_high > 0);   k_high--);

    criterion   = 0.0;
    nThreshold  = 127;
    fMu_0       = 126.0;
    fMu_1       = 128.0;

    omega_k     = 0.0;
    fMu_k       = 0.0;
    //minimize S_W/S_B to find the treshold
    for (int k = k_low ; k <= k_high ; k++) {
        omega_k += hist[k];

        if( omega_k == 0 || omega_k == 1 ) continue;

        fMu_k      += k * hist[k];
        expr_1      = ( fMu_T * omega_k - fMu_k );
        sigma_b_k   = expr_1 * expr_1 / ( omega_k * ( 1 - omega_k ));

        if (  criterion < sigma_b_k / sigma_T ) {
            criterion   = sigma_b_k / sigma_T;
            nThreshold  = k;
            fMu_0       = fMu_k / omega_k;
            fMu_1       = (fMu_T-fMu_k) / ( 1 - omega_k );
        }
    }
    fMu = fMu_T;

    return nThreshold;
}

const QImage& CImage::binarize(int t)
{
    t = t < 0 ? threshold : t;

    imgBinary = QImage(imgGray.size(), QImage::Format_Indexed8);
    imgBinary.setColorTable(graytable);

    quint8 * p1 = imgGray.bits();
    quint8 * p2 = imgBinary.bits();

    int i, n = imgGray.width() * imgGray.height();
    for(i = 0; i < n; ++i, ++p1, ++p2){
        *p2 = *p1 > t ? 255 : 0;
    }

    return imgBinary;
}
