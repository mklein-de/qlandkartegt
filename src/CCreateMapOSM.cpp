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

CCreateMapOSM::CCreateMapOSM(QWidget * parent)
    : QWidget(parent)
    , dataset(0)
    , band(0)
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
    if(dataset) delete dataset;
}

void CCreateMapOSM::slotCreate()
{
    QString filename = QDir(labelPath->text()).filePath(lineName->text());

    float lon1 = 0, lat1 = 0, lon2 = 0, lat2 = 0;
    GPS_Math_Str_To_Deg(lineTopLeft->text(), lon1, lat1);
    GPS_Math_Str_To_Deg(lineBottomRight->text(), lon2, lat2);

    zoomlevels.clear();
    addZoomLevel(13, lon1, lat1, lon2, lat2);
    addZoomLevel(11, lon1, lat1, lon2, lat2);
    addZoomLevel(9, lon1, lat1, lon2, lat2);

//
//     // for 17th zoom level
//     idxZoom = 0;
//     x1 = (lon1 + 180) * (1<<zoomlevels[idxZoom]) / 360;
//     x2 = (lon2 + 180) * (1<<zoomlevels[idxZoom]) / 360;
//
//     y1 = (1 - log(tan(lat1 * PI / 180) + 1 / cos(lat1 * PI / 180)) / PI) / 2 * (1<<zoomlevels[idxZoom]);
//     y2 = (1 - log(tan(lat2 * PI / 180) + 1 / cos(lat2 * PI / 180)) / PI) / 2 * (1<<zoomlevels[idxZoom]);
//
//     link->setHost("tah.openstreetmap.org");
//
//     idxZoom = 0;
//     while(zoomlevels[idxZoom] != 0){
//
//
//         ++idxZoom;
//     }
//
//     // setup GDAL GeoTiff image
//     GDALDriverManager * drvman = GetGDALDriverManager();
//     GDALDriver *        driver = drvman->GetDriverByName("GTiff");
//
//     if(dataset) delete dataset;
//     dataset = driver->Create("test.tif",(x2 - x1 + 1) * 256,(y2 - y1 + 1) * 256,1,GDT_Byte,osm_image_args);
//     band    = dataset->GetRasterBand(1);
//     band->SetColorTable(&gdalColorTable);
//
//     x = x1;
//     y = y1;

//     getNextTile();
}

void CCreateMapOSM::getNextTile()
{

//     QUrl url;
//     url.setPath(QString("/Tiles/tile.php/%1/%2/%3.png").arg(zoomlevels[idxZoom]).arg(x).arg(y));
//     qDebug() << url;
//     link->get(url.toEncoded( ));
}

void CCreateMapOSM::addZoomLevel(int zoom, float lon1, float lat1, float lon2, float lat2)
{
    int x = 0, y = 0;
    zoomlevel_t z(zoom);

    x1 = (lon1 + 180) * (1<<zoom) / 360;
    x2 = (lon2 + 180) * (1<<zoom) / 360;

    y1 = (1 - log(tan(lat1 * PI / 180) + 1 / cos(lat1 * PI / 180)) / PI) / 2 * (1<<zoom);
    y2 = (1 - log(tan(lat2 * PI / 180) + 1 / cos(lat2 * PI / 180)) / PI) / 2 * (1<<zoom);

    GDALDriverManager * drvman = GetGDALDriverManager();
    GDALDriver *        driver = drvman->GetDriverByName("GTiff");

    QString filename = QString("%1%2.tif").arg(QDir(labelPath->text()).filePath(lineName->text())).arg(zoomlevels.count());
    z.dataset = driver->Create(filename.toLatin1(),(x2 - x1 + 1) * 256,(y2 - y1 + 1) * 256,1,GDT_Byte,osm_image_args);
    z.band    = dataset->GetRasterBand(1);
    z.band->SetColorTable(&gdalColorTable);

    zoomlevels << z;
    for(y = y1; y <= y2; ++y){
        for(x = x1; x <= x2; ++x){
            tile_t t;
            t.url.setPath(QString("/Tiles/tile.php/%1/%2/%3.png").arg(zoom).arg(x).arg(y));
            t.x         = x;
            t.y         = y;
            t.zoom      = zoom;
            t.zoomlevel = &zoomlevels.last();

            tiles << t;
        }
    }
}

void CCreateMapOSM::slotRequestFinished(int id, bool error)
{
    qDebug() << "slotRequestFinished(" <<  id << "," << error << ")";

//     QImage img1,img2;
//     img1.loadFromData(link->readAll());
//     if(img1.format() == QImage::Format_Invalid){
//         return;
//     }
//
//     img2 = img1.convertToFormat(QImage::Format_Indexed8,qtColorTable);
//     qDebug() << img2.format();
//     band->WriteBlock(x - x1, y - y1, img2.bits());
//
//     if(++x > x2){
//         x = x1;
//         if(++y > y2){
//             qDebug() << "done";
//             dataset->FlushCache();
//             delete dataset;
//             dataset = 0;
//             band    = 0;
//             return;
//         }
//     }
//
//     getNextTile();
}
