/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-830551
  Fax:         +49-941-83055-79

  File:        CDlgDeviceExportPath.h

  Module:

  Description:

  Created:     05/17/2012

  (C) 2012 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CDLGDEVICEEXPORTPATH_H
#define CDLGDEVICEEXPORTPATH_H

#include "ui_IDlgDeviceExportPath.h"
#include <QDialog>

class QDir;
class QString;

class CDlgDeviceExportPath : public QDialog, private Ui::IDlgDeviceExportPath
{
    Q_OBJECT;
    public:
        CDlgDeviceExportPath(const QString &what, QDir &dir, QString &subdir, QWidget *parent);
        virtual ~CDlgDeviceExportPath();

    private slots:
        void slotItemClicked(QListWidgetItem*item);
        void slotReturnPressed();

    private:
        QString& subdir;

};

#endif //CDLGDEVICEEXPORTPATH_H

