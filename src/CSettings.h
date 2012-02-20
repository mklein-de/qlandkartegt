/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/  

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CSettings.h

  Module:      

  Description:

  Created:     02/20/2012

  (C) 2012 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CSETTINGS_H
#define CSETTINGS_H

#include "CAppOpts.h"
#include <QtCore>

class CSettings : public QObject
{

    public:
        CSettings()
        {
            if(!qlOpts->configfile.isEmpty())
            {
                cfg = new QSettings(qlOpts->configfile, QSettings::IniFormat, this);
            }
            else
            {
                cfg = new QSettings(this);
            }
        }
        ~CSettings(){}

        QSettings& get(){return *cfg;}

    private:
        QSettings  * cfg;
};

#define SETTINGS \
    CSettings ccfg;\
    QSettings& cfg = ccfg.get()

#endif //CSETTINGS_H

