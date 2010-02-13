#ifndef CMAP3D_H
#define CMAP3D_H


#include <QGLWidget>

class CMap3D : public QGLWidget
{
    Q_OBJECT;
    public:
        CMap3D(QWidget *parent);
        virtual ~CMap3D();
};

#endif //CMAP3D_H

