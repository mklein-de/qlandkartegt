/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/  

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CDlgMapWmsConfig.cpp

  Module:      

  Description:

  Created:     01/12/2012

  (C) 2012 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "CDlgMapWmsConfig.h"
#include "CMapWms.h"

#include <QtGui>

CDlgMapWmsConfig::CDlgMapWmsConfig(CMapWms &map)
    : map(map)
{

    setupUi(this);

    QTreeWidgetItem * item;
    item = new QTreeWidgetItem(treeMapConfig);
    item->setText(eColProperty, tr("Name"));
    item->setText(eColValue, map.name);
    item = new QTreeWidgetItem(treeMapConfig);
    item->setText(eColProperty, tr("Copyright"));
    item->setText(eColValue, map.copyright);
    item = new QTreeWidgetItem(treeMapConfig);
    item->setText(eColProperty, tr("URL"));
    item->setText(eColValue, map.urlstr);
    item = new QTreeWidgetItem(treeMapConfig);
    item->setText(eColProperty, tr("Format"));
    item->setText(eColValue, map.format);
    item = new QTreeWidgetItem(treeMapConfig);
    item->setText(eColProperty, tr("Layers"));
    item->setText(eColValue, map.layers);
    item = new QTreeWidgetItem(treeMapConfig);
    item->setText(eColProperty, tr("SRS"));
    item->setText(eColValue, map.srs);
    item = new QTreeWidgetItem(treeMapConfig);
    item->setText(eColProperty, tr("SizeX"));
    item->setText(eColValue, QString::number(map.xsize_px));
    item = new QTreeWidgetItem(treeMapConfig);
    item->setText(eColProperty, tr("SizeY"));
    item->setText(eColValue, QString::number(map.ysize_px));
    item = new QTreeWidgetItem(treeMapConfig);
    item->setText(eColProperty, tr("BlockSizeX"));
    item->setText(eColValue, QString::number(map.blockSizeX));
    item = new QTreeWidgetItem(treeMapConfig);
    item->setText(eColProperty, tr("BlockSizeY"));
    item->setText(eColValue, QString::number(map.blockSizeY));

}

CDlgMapWmsConfig::~CDlgMapWmsConfig()
{

}

