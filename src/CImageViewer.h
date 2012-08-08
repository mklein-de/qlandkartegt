
#ifndef CIMAGEVIEWER_H
#define CIMAGEVIEWER_H

#include <QDialog>
#include "ui_IImageViewer.h"
#include "CWpt.h"

class CImageViewer : public QDialog, private Ui::IImageViewer
{
    Q_OBJECT;
    public:
        CImageViewer(QList<CWpt::image_t> &images, QWidget *parent);
        virtual ~CImageViewer();
};

#endif //CIMAGEVIEWER_H

