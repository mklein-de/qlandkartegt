/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  Fax:         +49-941-83055-79

  File:        CSearch.h

  Module:

  Description:

  Created:     09/03/2008

  (C) 2008 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CSEARCH_H
#define CSEARCH_H

#include <QObject>

class CSearch : public QObject
{
    Q_OBJECT;
    public:
        CSearch(QObject * parent);
        virtual ~CSearch();

        qreal   lon;
        qreal   lat;
        QString query;
};

#endif //CSEARCH_H

