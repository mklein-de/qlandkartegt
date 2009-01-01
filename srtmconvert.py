#!python

import os
from string import join

geotifs = []

for root, dirs, files in os.walk("./", topdown=False):
    for name in files:

        if os.path.splitext(name)[1] != ".hgt": continue

        filename = os.path.join(root, name)
        cmd = "gdalwarp -t_srs \"+proj=merc +lon_0=0 +k=1 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs \" -r cubic %s %s.tif" % (filename, name[:-4])
        os.system(cmd)

        geotifs.append(name[:-4] + ".tif")

cmd = "gdalwarp -r cubic " + join(geotifs) + " tmp.tif"
os.system(cmd)

cmd = "gdal_translate -co tiled=yes -co blockxsize=256 -co blockysize=256 -co compress=deflate -co predictor=1 tmp.tif srtm.tif"
os.system(cmd)

