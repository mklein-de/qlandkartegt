
#ifndef CIMAGEVIEWER_H
#define CIMAGEVIEWER_H

#include <QDialog>
#include "ui_IImageViewer.h"
#include "CWpt.h"

class CImageViewer : public QDialog, private Ui::IImageViewer
{
    Q_OBJECT;
    public:
        CImageViewer(QList<CWpt::image_t> &images, int idx, QWidget *parent);

        virtual ~CImageViewer();

    protected:
        void paintEvent(QPaintEvent * e);
        void resizeEvent(QResizeEvent * e);
        void mousePressEvent(QMouseEvent * e);


    private:
        void setImageAtIdx(int i);

        QList<CWpt::image_t> images;

        int idx;

        QRect rectImage;
        QRect rectClose;
        QRect rectPrev;
        QRect rectNext;
};

#endif //CIMAGEVIEWER_H

