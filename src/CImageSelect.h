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

#endif //CIMAGESELECT_H

