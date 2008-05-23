/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  FAX:         +49-941-83055-79

  File:        CLiveLog.cpp

  Module:

  Description:

  Created:     05/20/2008

  (C) 2008


**********************************************************************************************/

#include "CLiveLog.h"


void operator <<(QFile& f, const CLiveLog& log)
{
    f.open(QIODevice::Append);
    QDataStream s(&f);
    s << log;
    f.close();
}

void operator <<(QDataStream& s, const CLiveLog& log)
{
    s << log.timestamp;
    s << log.lon;
    s << log.lat;
    s << log.ele;
    s << (quint32)0; // terminator, non-zero defines additional data
//     s << (quint32)0; // sizeof additional data in bytes if terminator is non-zero
}

void operator >>(QDataStream& s, CLiveLog& log)
{
    quint32 dummy;
    s >> log.timestamp;
    s >> log.lon;
    s >> log.lat;
    s >> log.ele;
    s >> dummy;

}


CLiveLog::~CLiveLog()
{

}

