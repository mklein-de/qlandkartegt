#!/usr/bin/python
#
# Augment cmake generated Info.plist with file formats understood by QLGT
# 
# Written 2011 by Michael Klein <michael.klein@puffin.lb.shuttle.de>
# Released under the terms of the GPL v. 2 as usual.

import sys
from Foundation import *

if len(sys.argv) != 2:
    print >> sys.stderr, "Usage: %s <bundle>" % sys.argv[0]
    exit(1)

info_plist = NSDictionary.dictionaryWithContentsOfFile_(sys.argv[1] + "/Contents/Info.plist")
if info_plist == None:
    print >> sys.stderr, "Fatal: can't load <%s>" % sys.argv[1]
    exit(1)

info_plist.setObject_forKey_("QLGT", "CFBundleSignature")

btd = NSMutableArray.array()

types = { "gpx": "GPS exchange data",
          "qlb": "QLandkarte data",
          "tcx": "TrainingsCenterExchange data",
          "gdb": "MapSource data",
          "kml": "Google Earth (Keyhole) data",
          "loc": "Geocaching.com - EasyGPS data",
          "plt": "OziExplorer track",
          "rte": "OziExplorer route",
          "wpt": "OziExplorer waypoint"
          }

for ext, name in types.iteritems():
    dt = NSMutableDictionary.dictionary()
    dt.setObject_forKey_(NSArray.arrayWithObject_(ext), "CFBundleTypeExtensions")
    dt.setObject_forKey_(name, "CFBundleTypeName")
    # icons produced by http://code.google.com/p/docerator/
    dt.setObject_forKey_("qlandkartegt-%s.icns" % ext, "CFBundleTypeIconFile")
    btd.append(dt)

info_plist.setObject_forKey_(btd, "CFBundleDocumentTypes")
info_plist.writeToFile_atomically_(sys.argv[1] + "/Contents/Info.plist", True)
