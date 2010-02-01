/**********************************************************************************************
    Copyright (C) 2009 Oliver Eichler oliver.eichler@gmx.de

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/

#include "CDlgCreateWorldBasemap.h"
#include "COsmTilesHash.h"
#include <gdal.h>

#include <math.h>

#include <QtGui>

static const GDALColorEntry defaultColorTable[256] =
{
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

static QVector<QRgb>  qtColorTable(256);

CDlgCreateWorldBasemap::CDlgCreateWorldBasemap()
{
    setupUi(this);
    tilehash = new COsmTilesHash("tile.openstreetmap.org/%1/%2/%3.png");
    connect(tilehash, SIGNAL(newImageReady(QImage, bool)), this, SLOT(slotImageReady(QImage, bool)));
    connect(spinLevel, SIGNAL(valueChanged(int)), this, SLOT(slotChangeLevel(int)));

    for(int i=0; i<256; ++i)
    {
        qtColorTable[i] = qRgb(defaultColorTable[i].c1,defaultColorTable[i].c2,defaultColorTable[i].c3);
    }

}


CDlgCreateWorldBasemap::~CDlgCreateWorldBasemap()
{

}


void CDlgCreateWorldBasemap::slotChangeLevel(int val)
{
    int width = pow(2.0, val - 1) * 256;
    labelDimensions->setText(QString("%1px x %1px\n %2 MB uncompressed").arg(width).arg((width * width * 4)/1048576));
}


void CDlgCreateWorldBasemap::accept()
{
    int level = spinLevel->value() - 1;
    int width = pow(2.0, level) * 256;

    tilehash->startNewDrawing(-180, 85.0511, level, QRect(0,0,width,width));
}


void CDlgCreateWorldBasemap::slotImageReady(QImage image, bool lastTileLoaded)
{
    labelPreview->setPixmap(QPixmap::fromImage(image.scaled(512,512)));

    if(lastTileLoaded)
    {
        QImage tmp = image.convertToFormat(QImage::Format_Indexed8,qtColorTable);
        tmp.save("./basemap.png");
        QStringList args;
        args << "-gcp" << "0" << "0" << "-20037508" << "20037508";
        args << "-gcp" << QString("%1").arg(image.width()) << "0"  << "20037508"  << "20037508";
        args << "-gcp" << "0" << QString("%1").arg(image.height()) << "-20037508" << "-20037508";
        args << "-a_srs" << "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs";
        args << "./basemap.png" << "./basemap.tif";
        qDebug() << "gdal_translate" << args.join(" ");
        QProcess::execute("gdal_translate", args);

        args.clear();
        args << "-t_srs" << "EPSG:4326";
        args << "-ts" << QString("%1").arg(image.width()) << QString("%1").arg(image.height());
        args << "./basemap.tif" << "./basemap_wgs84.tif";
        qDebug() << "gdalwarp" << args.join(" ");
        QProcess::execute("gdalwarp", args);

        args.clear();
        args << "-co" << "tiled=yes";
        args << "-co" << "blockxsize=256";
        args << "-co" << "blockysize=256";
        args << "-co" << "compress=LZW";
        args << "./basemap_wgs84.tif" << "./basemap_wgs84_x.tif";
        qDebug() << "gdal_translate" << args.join(" ");
        QProcess::execute("gdal_translate", args);

    }
}
