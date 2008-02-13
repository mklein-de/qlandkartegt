/**********************************************************************************************

  DSP Solutions
  Ingenieure Kellermann, Voigt, Hoepfl, Eichler und Weidner, Partnerschaft
  http://www.dspsolutions.de/

  Author:      Not defined
  Email:       Not defined
  Phone:       Not defined
  FAX:         +49-941-83055-79

  File:        CMapDB.h

  Module:

  Description:

  Created:     02/13/2008

  (C) 2008


**********************************************************************************************/
#ifndef CMAPDB_H
#define CMAPDB_H

#include "IDB.h"



class CMapDB : public IDB
{
    Q_OBJECT
    public:
        virtual ~CMapDB();

        static CMapDB& self(){return *m_self;}

        void loadGPX(CGpx& gpx);
        void saveGPX(CGpx& gpx);

        void loadQLB(CQlb& qlb);
        void saveQLB(CQlb& qlb);

        void upload();
        void download();


    private:
        friend class CMainWindow;

        CMapDB(QToolBox * tb, QObject * parent);

        static CMapDB * m_self;
};

#endif //CMAPDB_H

