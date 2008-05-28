/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  FAX:         +49-941-83055-79

  File:        CMapSelection.h

  Module:

  Description:

  Created:     05/28/2008

  (C) 2008


**********************************************************************************************/
#ifndef CMAPSELECTION_H
#define CMAPSELECTION_H

#include <QString>

class CMapSelection
{
    public:
        CMapSelection() : lon1(0), lat1(0), lon2(0), lat2(0) {};

        static QString focusedMap;
        QString key;
        QString mapkey;
        QString description;
        double lon1; ///< top left longitude [rad]
        double lat1; ///< top left latitude [rad]
        double lon2; ///< bottom right longitude [rad]
        double lat2; ///< bottom right latitude [rad]


};

#endif //CMAPSELECTION_H

