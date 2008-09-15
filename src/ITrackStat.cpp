/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        ITrackStat.cpp

  Module:

  Description:

  Created:     09/15/2008

  (C) 2008 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "ITrackStat.h"

#define SPACING 9

ITrackStat::ITrackStat(QWidget * parent)
: QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setupUi(this);

    layout()->setSpacing(SPACING);

}

ITrackStat::~ITrackStat()
{

}

