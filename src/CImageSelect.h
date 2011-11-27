/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        CImageSelect.h

  Module:

  Description:

  Created:     11/26/2011

  (C) 2011 DSP Solutions. All rights reserved.


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


    signals:
        void sigChangedImage(int);

    protected:
        void paintEvent(QPaintEvent * e);
        void resizeEvent(QResizeEvent * e);
        void mousePressEvent(QMouseEvent * e);
        void wheelEvent(QWheelEvent * e);

        struct img_t
        {
            img_t(const QString& title, const QString& src)
                : img(src)
                , title(title)
            {

            }

            QPixmap img;
            QString title;
        };

        QList<img_t> images;
        CWpt * wpt;
};

#endif //CIMAGESELECT_H

