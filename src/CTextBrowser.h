/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CTextBrowser.h

  Module:

  Description:

  Created:     06/20/2012

  (C) 2012 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CTEXTBROWSER_H
#define CTEXTBROWSER_H

#include <QTextBrowser>

class CTextBrowser : public QTextBrowser
{
    Q_OBJECT;
    public:
        CTextBrowser(QWidget * parent);
        virtual ~CTextBrowser();

        void addArea(const QString& key, const QRect& rect);

        void highlightArea(const QString& key);

    protected:
        void paintEvent(QPaintEvent * e);

    private:

        QMap<QString, QRect> areas;

        QString areaKey;
};

#endif //CTEXTBROWSER_H

