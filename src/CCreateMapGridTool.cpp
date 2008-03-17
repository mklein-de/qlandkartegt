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

#include "CCreateMapGridTool.h"
#include "CCreateMapGeoTiff.h"
#include "CMainWindow.h"
#include "CCanvas.h"
#include "CMapDB.h"

#include <QtGui>

#include <gdal.h>

CCreateMapGridTool::CCreateMapGridTool(CCreateMapGeoTiff * geotifftool, QWidget * parent)
: QWidget(parent)
, geotifftool(geotifftool)
{
    setupUi(this);

    labelStep2a->setPixmap(QPixmap(":/pics/Step2a"));
    labelStep2b->setPixmap(QPixmap(":/pics/Step2b"));
    labelStep2c->setPixmap(QPixmap(":/pics/Step2c"));
    labelExample->setPixmap(QPixmap(":/pics/grid_example"));

    connect(pushCancel, SIGNAL(clicked()), this, SLOT(deleteLater()));
    connect(pushOk, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(lineLongitude, SIGNAL(editingFinished()), this, SLOT(slotCheck()));
    connect(lineLatitude, SIGNAL(editingFinished()), this, SLOT(slotCheck()));
    connect(lineXSpacing, SIGNAL(editingFinished()), this, SLOT(slotCheck()));
    connect(lineYSpacing, SIGNAL(editingFinished()), this, SLOT(slotCheck()));

    QWidget * mapedit = theMainWindow->findChild<QWidget*>("CMapEditWidget");
    if(mapedit){
        mapedit->hide();
    }


    QSettings cfg;
    lineProjection->setText(cfg.value("create/ref.proj","").toString());
    lineXSpacing->setText(cfg.value("create/grid.x.spacing","1000").toString());
    lineYSpacing->setText(cfg.value("create/grid.y.spacing","1000").toString());


    for(int i = 1;  i != 5; ++i){

        CCreateMapGeoTiff::refpt_t& pt = geotifftool->refpts[-i];
        pt.item         = new QTreeWidgetItem();

        pt.item->setData(CCreateMapGeoTiff::eLabel,Qt::UserRole,-i);
        pt.item->setText(CCreateMapGeoTiff::eLabel,tr("%1").arg(i));
        pt.item->setText(CCreateMapGeoTiff::eLonLat,tr(""));


        QPoint center   = theMainWindow->getCanvas()->rect().center();
        IMap& map       = CMapDB::self().getMap();
        switch(i){
            case 1:
                pt.x            = center.x() - 50;
                pt.y            = center.y();
                break;
            case 2:
                pt.x            = center.x() + 50;
                pt.y            = center.y();
                break;
            case 3:
                pt.x            = center.x() + 50;
                pt.y            = center.y() + 100;
                break;
            case 4:
                pt.x            = center.x() - 50;
                pt.y            = center.y() + 100;
                break;
        }
        map.convertPt2M(pt.x,pt.y);
    }
    theMainWindow->getCanvas()->update();

    slotCheck();
}

CCreateMapGridTool::~CCreateMapGridTool()
{

    QSettings cfg;
    cfg.setValue("create/ref.proj",lineProjection->text());
    cfg.setValue("create/grid.x.spacing",lineXSpacing->text());
    cfg.setValue("create/grid.y.spacing",lineYSpacing->text());

    for(int i = 1;  i != 5; ++i){
        CCreateMapGeoTiff::refpt_t& pt = geotifftool->refpts[-i];
        if(pt.item) delete pt.item;
        geotifftool->refpts.remove(-i);
    }

    QWidget * mapedit = theMainWindow->findChild<QWidget*>("CMapEditWidget");
    if(mapedit){
        mapedit->show();
    }
}

void CCreateMapGridTool::slotCheck()
{
    pushOk->setEnabled(false);
    labelStep2c->setEnabled(false);
    if(lineLongitude->text().isEmpty()) return;
    if(lineLatitude->text().isEmpty()) return;
    if(lineXSpacing->text().isEmpty()) return;
    if(lineYSpacing->text().isEmpty()) return;
    pushOk->setEnabled(true);
    labelStep2c->setEnabled(true);

}

void CCreateMapGridTool::slotOk()
{
    int res;
    double adfGeoTransform1[6], adfGeoTransform2[6];
    double northing = lineLatitude->text().toDouble();
    double easting  = lineLongitude->text().toDouble();
    double stepx    = lineXSpacing->text().toDouble();
    double stepy    = lineYSpacing->text().toDouble();

    CCreateMapGeoTiff::refpt_t& pt1 = geotifftool->refpts[-1];
    CCreateMapGeoTiff::refpt_t& pt2 = geotifftool->refpts[-2];
    CCreateMapGeoTiff::refpt_t& pt3 = geotifftool->refpts[-3];
    CCreateMapGeoTiff::refpt_t& pt4 = geotifftool->refpts[-4];

    GDAL_GCP gcps[4];
    memset(gcps,0,sizeof(gcps));
    gcps[0].dfGCPPixel  = pt1.x;
    gcps[0].dfGCPLine   = pt1.y;
    gcps[0].dfGCPX      = easting;
    gcps[0].dfGCPY      = northing;

    gcps[1].dfGCPPixel  = pt2.x;
    gcps[1].dfGCPLine   = pt2.y;
    gcps[1].dfGCPX      = easting + stepx;
    gcps[1].dfGCPY      = northing;

    gcps[2].dfGCPPixel  = pt3.x;
    gcps[2].dfGCPLine   = pt3.y;
    gcps[2].dfGCPX      = easting + stepx;
    gcps[2].dfGCPY      = northing - stepy;

    gcps[3].dfGCPPixel  = pt4.x;
    gcps[3].dfGCPLine   = pt4.y;
    gcps[3].dfGCPX      = easting;
    gcps[3].dfGCPY      = northing - stepy;

    res = GDALGCPsToGeoTransform(4,gcps,adfGeoTransform1,TRUE);
    if(res == FALSE){
        QMessageBox::warning(0,tr("Error ..."), tr("Failed to calculate transformation for ref. points. Are all 4 points placed propperly?"), QMessageBox::Abort,QMessageBox::Abort);
        return;
    }
//     qDebug() << "adfGeoTransform1[0] = " << adfGeoTransform1[0];
//     qDebug() << "adfGeoTransform1[1] = " << adfGeoTransform1[1];
//     qDebug() << "adfGeoTransform1[2] = " << adfGeoTransform1[2];
//     qDebug() << "adfGeoTransform1[3] = " << adfGeoTransform1[3];
//     qDebug() << "adfGeoTransform1[4] = " << adfGeoTransform1[4];
//     qDebug() << "adfGeoTransform1[5] = " << adfGeoTransform1[5];
//     qDebug();
    GDALInvGeoTransform(adfGeoTransform1, adfGeoTransform2);
//     qDebug() << "adfGeoTransform2[0] = " << adfGeoTransform2[0];
//     qDebug() << "adfGeoTransform2[1] = " << adfGeoTransform2[1];
//     qDebug() << "adfGeoTransform2[2] = " << adfGeoTransform2[2];
//     qDebug() << "adfGeoTransform2[3] = " << adfGeoTransform2[3];
//     qDebug() << "adfGeoTransform2[4] = " << adfGeoTransform2[4];
//     qDebug() << "adfGeoTransform2[5] = " << adfGeoTransform2[5];
//     qDebug();

    PJ * pjWGS84 = 0, * pjSrc = 0;
    if(!lineProjection->text().isEmpty()){
        pjSrc   = pj_init_plus(lineProjection->text().toLatin1());
        if(pjSrc == 0){
            QMessageBox::warning(0,tr("Error ..."), tr("Failed to setup projection. Bad syntax?"), QMessageBox::Abort,QMessageBox::Abort);
            return;
        }
        pjWGS84 = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
    }

    double u1 = ((int)((adfGeoTransform1[0] + stepx) / stepx)) * stepx;
    double v1 = ((int)(adfGeoTransform1[3] / stepy)) * stepy;

    QRect rect(0,0,geotifftool->sizeOfInputFile.width(),geotifftool->sizeOfInputFile.height());

    double u = u1, v = v1;
    double x = 0,  y = 0;
    bool go         = true;
    bool validLine  = false;
    while(go){
        GDALApplyGeoTransform(adfGeoTransform2, u, v, &x, &y);
        if(rect.contains(QPoint(x,y))){
            // add ref point
            double _u = u;
            double _v = v;
            if(pjSrc){
                pj_transform(pjSrc,pjWGS84,1,0,&_u,&_v,0);
                _u = _u * RAD_TO_DEG;
                _v = _v * RAD_TO_DEG;
            }

            geotifftool->addRef(x,y,_u,_v);
            validLine = true;
        }
        else{
            if(x > rect.width()){
                if(!validLine) break;
                validLine = false;

                u  = u1;
                v -= stepy;
                continue;
            }
        }

        u += stepx;
    }

    if(pjWGS84) pj_free(pjWGS84);
    if(pjSrc) pj_free(pjSrc);

    geotifftool->comboMode->setCurrentIndex(geotifftool->comboMode->findData(CCreateMapGeoTiff::eQuadratic));

    theMainWindow->getCanvas()->update();
    deleteLater();
}
