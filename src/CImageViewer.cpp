

#include "CImageViewer.h"

CImageViewer::CImageViewer(QList<CWpt::image_t>& images, QWidget *parent)
: QDialog(parent)
{
    setupUi(this);

    setWindowOpacity(0.75);
    Qt::WindowFlags flags = windowFlags() & Qt::WindowType_Mask;
    setWindowFlags(flags | Qt::CustomizeWindowHint);
    showMaximized();

    if(images.isEmpty())
    {
        return;
    }

    labelImage->setWindowOpacity(1.0);
    labelImage->setPixmap(images.first().pixmap);
}

CImageViewer::~CImageViewer()
{

}

