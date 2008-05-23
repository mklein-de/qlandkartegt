/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  FAX:         +49-941-83055-79

  File:        CLiveLog.h

  Module:

  Description:

  Created:     05/20/2008

  (C) 2008


**********************************************************************************************/
#ifndef CLIVELOG_H
#define CLIVELOG_H

#include <QtGlobal>
#include "CWpt.h"

class CLiveLog
{
    public:
        CLiveLog() : fix(eOff), lon(WPT_NOFLOAT), lat(WPT_NOFLOAT), ele(WPT_NOFLOAT)
                   , timestamp(0xFFFFFFFF), error_horz(WPT_NOFLOAT), error_vert(WPT_NOFLOAT)
                   , heading(WPT_NOFLOAT), velocity(WPT_NOFLOAT){};
        virtual ~CLiveLog();

        enum fix_e {eNoFix, e2DFix, e3DFix, eOff};

        fix_e fix;
        float lon;
        float lat;
        float ele;
        quint32 timestamp;
        float error_horz;
        float error_vert;
        float heading;
        float velocity;
};

extern void operator <<(QDataStream& s, const CLiveLog& log);
extern void operator <<(QFile& f, const CLiveLog& log);
extern void operator >>(QDataStream& s, CLiveLog& log);



#endif //CLIVELOG_H

