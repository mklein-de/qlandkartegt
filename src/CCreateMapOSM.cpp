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

#include <QtGui>
#include <QtNetwork/QHttp>
#include <math.h>

static char * osm_image_args[] = {
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
    lineName->setText("testosm");

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

}

CCreateMapOSM::~CCreateMapOSM()
{

}

void CCreateMapOSM::slotCreate()
{
    if(progress) return;
    QString filename = QDir(labelPath->text()).filePath(lineName->text());

    float lon1 = 0, lat1 = 0, lon2 = 0, lat2 = 0;
    GPS_Math_Str_To_Deg(lineTopLeft->text(), lon1, lat1);
    GPS_Math_Str_To_Deg(lineBottomRight->text(), lon2, lat2);

    // reset tiles
    tiles.clear();
    // reset zoomlevels
    zoomlevels.clear(); zoomlevels.resize(3);

    addZoomLevel(0, 14, lon1, lat1, lon2, lat2);
    addZoomLevel(1, 12, lon1, lat1, lon2, lat2);
    addZoomLevel(2, 10, lon1, lat1, lon2, lat2);

    maxTiles = tiles.count();
    progress= new QProgressDialog(tr("Download files ..."),tr("Abort"),0,tiles.count(),this);

    link->setHost("tah.openstreetmap.org");
    getNextTile();

}


void CCreateMapOSM::addZoomLevel(int level, int zoom, float lon1, float lat1, float lon2, float lat2)
{
    int x = 0, y = 0;

    zoomlevel_t& z = zoomlevels[level];

    int x1 = (lon1 + 180) * (1<<zoom) / 360;
    int x2 = (lon2 + 180) * (1<<zoom) / 360;

    int y1 = (1 - log(tan(lat1 * PI / 180) + 1 / cos(lat1 * PI / 180)) / PI) / 2 * (1<<zoom);
    int y2 = (1 - log(tan(lat2 * PI / 180) + 1 / cos(lat2 * PI / 180)) / PI) / 2 * (1<<zoom);

    GDALDriverManager * drvman = GetGDALDriverManager();
    GDALDriver *        driver = drvman->GetDriverByName("GTiff");

    QString filename = QString("%1%2.tif").arg(QDir(labelPath->text()).filePath(lineName->text())).arg(level);
    z.dataset = driver->Create(filename.toLatin1(),(x2 - x1 + 1) * 256,(y2 - y1 + 1) * 256,1,GDT_Byte,osm_image_args);
    z.band    = z.dataset->GetRasterBand(1);
    z.band->SetColorTable(&gdalColorTable);


    for(y = y1; y <= y2; ++y){
        for(x = x1; x <= x2; ++x){
            tile_t t;
            t.url.setPath(QString("/Tiles/tile.php/%1/%2/%3.png").arg(zoom).arg(x).arg(y));
            t.x         = x - x1;
            t.y         = y - y1;
            t.zoom      = zoom;
            t.zoomlevel = &z;

            qDebug() << t.x << t.y;

            tiles << t;
        }
    }

}

void CCreateMapOSM::getNextTile()
{
    if(progress && progress->wasCanceled()){
        tiles.clear();
        // TODO: unlink files!
        zoomlevels.clear();
        delete progress; progress = 0;
        return;
    }

    if(tiles.isEmpty()){
        // TODO: write *.qmap
        zoomlevels.clear();
        delete progress; progress = 0;
        return;
    }


    tile_t& t = tiles.first();
    progress->setValue(maxTiles - tiles.count());
    progress->setLabelText(tr("download: ") + t.url.toString());

    link->get(t.url.toEncoded());
}


void CCreateMapOSM::slotRequestFinished(int id, bool error)
{
    qDebug() << "slotRequestFinished(" <<  id << "," << error << ")";

    if(error){
        QMessageBox::critical(this,tr("Failed to download tile!"), link->errorString(), QMessageBox::Retry|QMessageBox::Abort, QMessageBox::Retry);
        return;
    }

    QImage img1,img2;
    img1.loadFromData(link->readAll());
    if(img1.format() == QImage::Format_Invalid){
        // that:
        // link->setHost("tah.openstreetmap.org")
        // will cause a requestFinished() signal, too
        qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxxx";
        return;
    }

    img2 = img1.convertToFormat(QImage::Format_Indexed8,qtColorTable);
    tile_t& t = tiles.first();

    qDebug() << t.x << t.y;
    t.zoomlevel->band->WriteBlock(t.x, t.y, img2.bits());

    if(tiles.count()) tiles.pop_front();

    getNextTile();

}
