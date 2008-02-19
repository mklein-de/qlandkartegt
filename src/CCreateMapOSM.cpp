/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de

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

#include "CCreateMapOSM.h"
#include "CResources.h"
#include "GeoMath.h"
#include "CMapDB.h"
#include "CMainWindow.h"

#include <QtGui>
#include <QtNetwork/QHttp>
#include <math.h>
#include <ogr_spatialref.h>

static const char * osm_image_args[] = {
     "BLOCKXSIZE=256"
    ,"BLOCKYSIZE=256"
    ,"TILED=YES"
    ,"COMPRESS=DEFLATE"
    ,NULL
};


static GDALColorEntry defaultColorTable[256] = {
    {0,0,0,0        },
    {32,0,0,0       },
    {65,0,0,0       },
    {106,0,0,0      },
    {-117,0,0,0     },
    {-76,0,0,0      },
    {-43,0,0,0      },
    {-1,0,0,0       },
    {0,48,0,0       },
    {32,48,0,0      },
    {65,48,0,0      },
    {106,48,0,0,    },
    {-117,48,0,0    },
    {-76,48,0,0     },
    {-43,48,0,0     },
    {-1,48,0,0      },
    {0,101,0,0      },
    {32,101,0,0     },
    {65,101,0,0     },
    {106,101,0,0    },
    {-117,101,0,0   },
    {-76,101,0,0    },
    {-43,101,0,0    },
    {-1,101,0,0     },
    {0,-107,0,0     },
    {32,-107,0,0    },
    {65,-107,0,0    },
    {106,-107,0,0   },
    {-117,-107,0,0  },
    {-76,-107,0,0   },
    {-43,-107,0,0   },
    {-1,-107,0,0    },
    {0,-54,0,0      },
    {32,-54,0,0     },
    {65,-54,0,0     },
    {106,-54,0,0    },
    {-117,-54,0,0   },
    {-76,-54,0,0    },
    {-43,-54,0,0    },
    {-1,-54,0,0     },
    {0,-1,0,0       },
    {32,-1,0,0      },
    {65,-1,0,0      },
    {106,-1,0,0     },
    {-117,-1,0,0    },
    {-76,-1,0,0     },
    {-43,-1,0,0     },
    {-1,-1,0,0      },
    {0,0,57,0       },
    {32,0,57,0      },
    {65,0,57,0      },
    {106,0,57,0     },
    {-117,0,57,0    },
    {-76,0,57,0     },
    {-43,0,57,0     },
    {-1,0,57,0      },
    {0,48,57,0      },
    {32,48,57,0     },
    {65,48,57,0     },
    {106,48,57,0    },
    {-117,48,57,0   },
    {-76,48,57,0    },
    {-43,48,57,0    },
    {-1,48,57,0     },
    {0,101,57,0     },
    {32,101,57,0    },
    {65,101,57,0    },
    {106,101,57,0   },
    {-117,101,57,0  },
    {-76,101,57,0   },
    {-43,101,57,0   },
    {-1,101,57,0    },
    {0,-107,57,0    },
    {32,-107,57,0   },
    {65,-107,57,0   },
    {106,-107,57,0  },
    {-117,-107,57,0 },
    {-76,-107,57,0  },
    {-43,-107,57,0  },
    {-1,-107,57,0   },
    {0,-54,57,0     },
    {32,-54,57,0    },
    {65,-54,57,0    },
    {106,-54,57,0   },
    {-117,-54,57,0  },
    {-76,-54,57,0   },
    {-43,-54,57,0   },
    {-1,-54,57,0    },
    {0,-1,57,0      },
    {32,-1,57,0     },
    {65,-1,57,0     },
    {106,-1,57,0    },
    {-117,-1,57,0   },
    {-76,-1,57,0    },
    {-43,-1,57,0    },
    {-1,-1,57,0     },
    {0,0,123,0      },
    {32,0,123,0     },
    {65,0,123,0     },
    {106,0,123,0    },
    {-117,0,123,0   },
    {-76,0,123,0    },
    {-43,0,123,0    },
    {-1,0,123,0     },
    {0,48,123,0     },
    {32,48,123,0    },
    {65,48,123,0    },
    {106,48,123,0   },
    {-117,48,123,0  },
    {-76,48,123,0   },
    {-43,48,123,0   },
    {-1,48,123,0    },
    {0,101,123,0    },
    {32,101,123,0   },
    {65,101,123,0   },
    {106,101,123,0  },
    {-117,101,123,0 },
    {-76,101,123,0  },
    {-43,101,123,0  },
    {-1,101,123,0   },
    {0,-107,123,0   },
    {32,-107,123,0  },
    {65,-107,123,0  },
    {106,-107,123,0 },
    {-117,-107,123,0},
    {-76,-107,123,0 },
    {-43,-107,123,0 },
    {-1,-107,123,0  },
    {0,-54,123,0    },
    {32,-54,123,0   },
    {65,-54,123,0   },
    {106,-54,123,0  },
    {-117,-54,123,0 },
    {-76,-54,123,0  },
    {-43,-54,123,0  },
    {-1,-54,123,0   },
    {0,-1,123,0     },
    {32,-1,123,0    },
    {65,-1,123,0    },
    {106,-1,123,0   },
    {-117,-1,123,0  },
    {-76,-1,123,0   },
    {-43,-1,123,0   },
    {-1,-1,123,0    },
    {0,0,-67,0      },
    {32,0,-67,0     },
    {65,0,-67,0     },
    {106,0,-67,0    },
    {-117,0,-67,0   },
    {-76,0,-67,0    },
    {-43,0,-67,0    },
    {-1,0,-67,0     },
    {0,48,-67,0     },
    {32,48,-67,0    },
    {65,48,-67,0    },
    {106,48,-67,0   },
    {-117,48,-67,0  },
    {-76,48,-67,0   },
    {-43,48,-67,0   },
    {-1,48,-67,0    },
    {0,101,-67,0    },
    {32,101,-67,0   },
    {65,101,-67,0   },
    {106,101,-67,0  },
    {-117,101,-67,0 },
    {-76,101,-67,0  },
    {-43,101,-67,0  },
    {-1,101,-67,0   },
    {0,-107,-67,0   },
    {32,-107,-67,0  },
    {65,-107,-67,0  },
    {106,-107,-67,0 },
    {-117,-107,-67,0},
    {-76,-107,-67,0 },
    {-43,-107,-67,0 },
    {-1,-107,-67,0  },
    {0,-54,-67,0    },
    {32,-54,-67,0   },
    {65,-54,-67,0   },
    {106,-54,-67,0  },
    {-117,-54,-67,0 },
    {-76,-54,-67,0  },
    {-43,-54,-67,0  },
    {-1,-54,-67,0   },
    {0,-1,-67,0     },
    {32,-1,-67,0    },
    {65,-1,-67,0    },
    {106,-1,-67,0   },
    {-117,-1,-67,0  },
    {-76,-1,-67,0   },
    {-43,-1,-67,0   },
    {-1,-1,-67,0    },
    {0,0,-1,0       },
    {32,0,-1,0      },
    {65,0,-1,0      },
    {106,0,-1,0     },
    {-117,0,-1,0    },
    {-76,0,-1,0     },
    {-43,0,-1,0     },
    {-1,0,-1,0      },
    {0,48,-1,0      },
    {32,48,-1,0     },
    {65,48,-1,0     },
    {106,48,-1,0    },
    {-117,48,-1,0   },
    {-76,48,-1,0    },
    {-43,48,-1,0    },
    {-1,48,-1,0     },
    {0,101,-1,0     },
    {32,101,-1,0    },
    {65,101,-1,0    },
    {106,101,-1,0   },
    {-117,101,-1,0  },
    {-76,101,-1,0   },
    {-43,101,-1,0   },
    {-1,101,-1,0    },
    {0,-107,-1,0    },
    {32,-107,-1,0   },
    {65,-107,-1,0   },
    {106,-107,-1,0  },
    {-117,-107,-1,0 },
    {-76,-107,-1,0  },
    {-43,-107,-1,0  },
    {-1,-107,-1,0   },
    {0,-54,-1,0     },
    {32,-54,-1,0    },
    {65,-54,-1,0    },
    {106,-54,-1,0   },
    {-117,-54,-1,0  },
    {-76,-54,-1,0   },
    {-43,-54,-1,0   },
    {-1,-54,-1,0    },
    {0,-1,-1,0      },
    {32,-1,-1,0     },
    {65,-1,-1,0     },
    {106,-1,-1,0    },
    {-117,-1,-1,0   },
    {-76,-1,-1,0    },
    {-43,-1,-1,0    },
    {-1,-1,-1,0     },
    {-1,-1,-1,0     },
    {-26,-26,-26,0  },
    {-43,-43,-43,0  },
    {-59,-59,-59,0  },
    {-76,-76,-76,0  },
    {-92,-92,-92,0  },
    {-108,-108,-108,0},
    {-125,-125,-125,0},
    {115,115,115,0  },
    {98,98,98,0     },
    {82,82,82,0     },
    {65,65,65,0     },
    {49,49,49,0     },
    {32,32,32,0     },
    {16,16,16,0     },
    {0,0,0,0        }
};

static GDALColorTable gdalColorTable;
static QVector<QRgb>  qtColorTable(256);

CCreateMapOSM::zoomlevel_t::~zoomlevel_t()
{
    if(dataset){
        qDebug() << "~zoomlevel_t()";
        dataset->FlushCache();
        delete dataset;
        dataset = 0;
    }

}
CCreateMapOSM::CCreateMapOSM(QWidget * parent)
    : QWidget(parent)
    , progress(0)
{
    setupUi(this);
    labelPath->setText(CResources::self().pathMaps);

    connect(pushCreate, SIGNAL(clicked()), this, SLOT(slotCreate()));

    QString url;
    quint16 port;
    bool enableProxy;

    enableProxy = CResources::self().getHttpProxy(url,port);

    link = new QHttp(this);
    if(enableProxy){
        link->setProxy(url,port);
    }

    connect(link,SIGNAL(requestFinished(int,bool)),this,SLOT(slotRequestFinished(int,bool)));

    // initialize color tables
    for(int i=0; i<256; ++i){
        gdalColorTable.SetColorEntry(i, &defaultColorTable[i]);
        qtColorTable[i] = qRgba(defaultColorTable[i].c1,defaultColorTable[i].c2,defaultColorTable[i].c3,defaultColorTable[i].c4);
    }

    connect(toolPath, SIGNAL(clicked()), this, SLOT(slotSelectPath()));

}

CCreateMapOSM::~CCreateMapOSM()
{

}

void CCreateMapOSM::slotCreate()
{
    if(!progress.isNull()) return;

    // sanity check
    if(lineTopLeft->text().isEmpty()){
        QMessageBox::critical(this,tr("Information missing ..."), tr("The top left coordinate is missing"), QMessageBox::Abort, QMessageBox::Abort);
        return;
    }
    if(lineBottomRight->text().isEmpty()){
        QMessageBox::critical(this,tr("Information missing ..."), tr("The bottom right coordinate is missing"), QMessageBox::Abort, QMessageBox::Abort);
        return;
    }
    if(lineName->text().isEmpty()){
        QMessageBox::critical(this,tr("Information missing ..."), tr("The map name is missing."), QMessageBox::Abort, QMessageBox::Abort);
        return;
    }
    if(lineComment->text().isEmpty()){
        QMessageBox::critical(this,tr("Information missing ..."), tr("The comment is missing."), QMessageBox::Abort, QMessageBox::Abort);
        return;
    }

    // get top left and bottom right coordinate
    float lon1 = 0, lat1 = 0, lon2 = 0, lat2 = 0;
    if(!GPS_Math_Str_To_Deg(lineTopLeft->text(), lon1, lat1)){
        return;
    }
    if(!GPS_Math_Str_To_Deg(lineBottomRight->text(), lon2, lat2)){
        return;
    }

    // if we reach this point we disable the GUI
    setEnabled(false);

    progress = new QProgressDialog(tr("Download files ..."),tr("Abort"),0,0,this);
    progress->setAttribute(Qt::WA_DeleteOnClose,true);
    progress->setAutoReset(false);
    progress->setAutoClose(false);
    progress->setLabelText(tr("Calculating tiles ..."));
    progress->setValue(0);

    qApp->processEvents();


    // build file base path/name for putput files
    QString filename = QDir(labelPath->text()).filePath(lineName->text());

    // reset tiles
    tiles.clear();
    // reset zoomlevels
    zoomlevels.clear(); zoomlevels.resize(3);

    // --------- write *.qmap -------------

    // calculate real width and height
    float a1 = 0, a2 = 0;
    XY p1,p2,p3,p4;
    p1.u = lon1 * DEG_TO_RAD;
    p1.v = lat1 * DEG_TO_RAD;
    p3.u = lon2 * DEG_TO_RAD;
    p3.v = lat2 * DEG_TO_RAD;
    p2.u = p3.u;
    p2.v = p1.v;
    p4.u = p1.u;
    p4.v = p3.v;

    int realWidth  = ::distance(p1, p2, a1, a2);
    int realHeight = ::distance(p1, p4, a1, a2);

    // write basic data and description
    QSettings mapdef(QString("%1.qmap").arg(QDir(labelPath->text()).filePath(lineName->text())),QSettings::IniFormat);
    mapdef.setValue("home/zoom",1);
    mapdef.setValue("home/center",lineTopLeft->text().replace("\260",""));

    mapdef.beginGroup("description");
    mapdef.setValue("comment",lineComment->text());

    QString str = lineTopLeft->text();
    str = str.replace("\260","");
    mapdef.setValue("topleft",str);

    str = lineBottomRight->text();
    str = str.replace("\260","");
    mapdef.setValue("bottomright",str);

    mapdef.setValue("width",QString("%1 m").arg(realWidth));
    mapdef.setValue("height",QString("%1 m").arg(realHeight));

    mapdef.endGroup(); // description

    mapdef.setValue("main/levels",3);

    // define zoomlevels, this will add tile definitions, too
    addZoomLevel(1, 17, lon1, lat1, lon2, lat2, mapdef);
    addZoomLevel(2, 14, lon1, lat1, lon2, lat2, mapdef);
    addZoomLevel(3, 11, lon1, lat1, lon2, lat2, mapdef);

    maxTiles = tiles.count();
    progress->setMaximum(maxTiles);


    link->setHost("tah.openstreetmap.org");
//     link->setHost("tile.openstreetmap.org");
    getNextTile();
}


void CCreateMapOSM::addZoomLevel(int level, int zoom, double lon1, double lat1, double lon2, double lat2, QSettings& mapdef)
{
    int x = 0, y = 0;
    char * ptr = 0;

    zoomlevel_t& z = zoomlevels[level -1];

    // calculate tile indices
    int x1 = (lon1 + 180) * (1<<zoom) / 360;
    int x2 = (lon2 + 180) * (1<<zoom) / 360;

    int y1 = (1 - log(tan(lat1 * PI / 180) + 1 / cos(lat1 * PI / 180)) / PI) / 2 * (1<<zoom);
    int y2 = (1 - log(tan(lat2 * PI / 180) + 1 / cos(lat2 * PI / 180)) / PI) / 2 * (1<<zoom);

    // greate GeoTiff dataset
    GDALDriverManager * drvman = GetGDALDriverManager();
    GDALDriver *        driver = drvman->GetDriverByName("GTiff");

    QString filename = QString("%1%2.tif").arg(QDir(labelPath->text()).filePath(lineName->text())).arg(level);
    z.dataset = driver->Create(filename.toLatin1(),(x2 - x1 + 1) * 256,(y2 - y1 + 1) * 256,1,GDT_Byte,(char**)osm_image_args);

    // write level information to *.qmap
    mapdef.beginGroup(QString("level%1").arg(level));
    mapdef.setValue("files",QString("%1%2.tif").arg(lineName->text()).arg(level));
    mapdef.setValue("zoomLevelMin", (level - 1) * 4 + 1);
    mapdef.setValue("zoomLevelMax", (level - 1) * 4 + 4);
    mapdef.endGroup();

    // setup projection
    OGRSpatialReference oSRS;
    oSRS.importFromProj4("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs");
    oSRS.exportToWkt(&ptr);
    z.dataset->SetProjection(ptr);
    CPLFree(ptr);

    // setup reference point
    double Ep   = -20037508.34 + (40075016.68 / (1<<zoom)) * x1;
    double Np   =  20037508.34 - (40075016.68 / (1<<zoom)) * y1;
//     double Np   =  20037471.21 - (40074942.42 / (1<<zoom)) * y1;
    double resx = (40075016.68 / ((1<<zoom) * 256));
    double resy = (40075016.68 / ((1<<zoom) * 256));

    double adfGeoTransform[6];
    adfGeoTransform[0] = Ep;                     /* top left x */
    adfGeoTransform[1] = resx;                   /* w-e pixel resolution */
    adfGeoTransform[2] = 0;                      /* rotation, 0 if image is "north up" */
    adfGeoTransform[3] = Np;                     /* top left y */
    adfGeoTransform[4] = 0;                      /* rotation, 0 if image is "north up" */
    adfGeoTransform[5] = -resy;                  /* n-s pixel resolution */
    z.dataset->SetGeoTransform(adfGeoTransform);

    // initialize raster band with default colortable
    z.band    = z.dataset->GetRasterBand(1);
    z.band->SetColorTable(&gdalColorTable);

    // add tile definitions to list
    for(y = y1; y <= y2; ++y){
        for(x = x1; x <= x2; ++x){
            tile_t t;
            t.url.setPath(QString("/Tiles/tile.php/%1/%2/%3.png").arg(zoom).arg(x).arg(y));
//             t.url.setPath(QString("/%1/%2/%3.png").arg(zoom).arg(x).arg(y));
            t.x         = x - x1;
            t.y         = y - y1;
            t.zoom      = zoom;
            t.zoomlevel = &z;

            tiles << t;
        }
    }
}

void CCreateMapOSM::getNextTile()
{
    if(progress && progress->wasCanceled()){
        // TODO: unlink files!
        finishJob();
        return;
    }

    if(tiles.isEmpty()){
        finishJob();
        QString fn = QString("%1.qmap").arg(QDir(labelPath->text()).filePath(lineName->text()));
        qDebug() << fn;
        CMapDB::self().openMap(fn, *theMainWindow->getCanvas());
        return;
    }

    tile_t& t = tiles.first();
    progress->setValue(maxTiles - tiles.count());
    QString msg = tr("download: ") + t.url.toString() + "\n";
    msg += tr("tile %1 of %2").arg(maxTiles - tiles.count() + 1).arg(maxTiles);
    progress->setLabelText(msg);

    link->get(t.url.toEncoded());
    qDebug() << ("http://tile.openstreetmap.org" + t.url.toString());
}

void CCreateMapOSM::finishJob()
{
    tiles.clear();
    zoomlevels.clear();
    setEnabled(true);
    if(!progress.isNull()) progress->close();
}

void CCreateMapOSM::slotRequestFinished(int id, bool error)
{
    qDebug() << "slotRequestFinished(" <<  id << "," << error << ")";

    if(error){
        QMessageBox::StandardButton res = QMessageBox::critical(this,tr("Failed to download tile!"), link->errorString(), QMessageBox::Retry|QMessageBox::Abort, QMessageBox::Retry);
        if(res == QMessageBox::Retry){
            getNextTile();
            return;
        }
        finishJob();
        return;
    }

    QImage img1,img2;
    img1.loadFromData(link->readAll());
    if(img1.format() == QImage::Format_Invalid){
        // that:
        // link->setHost("tah.openstreetmap.org")
        // will cause a requestFinished() signal, too.
        // let's ignore it
        return;
    }

    // convert image from 32bit png to indexed 8bit with default colortable
    img2 = img1.convertToFormat(QImage::Format_Indexed8,qtColorTable);
    tile_t& t = tiles.first();

    // add converted image to raster band
    t.zoomlevel->band->WriteBlock(t.x, t.y, img2.bits());

    if(tiles.count()) tiles.pop_front();
    getNextTile();

}


void CCreateMapOSM::slotSelectPath()
{
    QString path = QFileDialog::getExistingDirectory(this,tr("Select output path ..."), labelPath->text());
    if(!path.isEmpty()){
        labelPath->setText(path);
    }
}
