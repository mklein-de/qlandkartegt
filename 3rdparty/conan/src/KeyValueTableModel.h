/*!
    Conan - Connection Analyzer for Qt
    Copyright (C) 2008 - 2011 Elmar de Koning, edekoning@gmail.com

    This file is part of Conan.

    Conan is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Conan is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Conan.  If not, see <http://www.gnu.org/licenses/>.
*/

/*!
    \file
    \brief Contains KeyValueTableModel related declarations
*/


#ifndef _KEYVALUETABLEMODEL__27_11_09__15_20_44__H_
#define _KEYVALUETABLEMODEL__27_11_09__15_20_44__H_


#include "ConanDefines.h"
#include <QtCore/QAbstractTableModel>
#include <QtCore/QMap>


namespace conan {

    class CONAN_LOCAL KeyValueTableModel : public QAbstractTableModel
    {
    public:
        KeyValueTableModel (QObject* inParent = 0);

        void SetClassInfo (const QMap <QString, QString>& inInfo);

        virtual int rowCount (const QModelIndex& inParent = QModelIndex ()) const;
        virtual int columnCount (const QModelIndex& inParent = QModelIndex ()) const;
        virtual QVariant data (const QModelIndex& inIndex, int inRole = Qt::DisplayRole) const;

    private:
        QMap <QString, QString> mData;
    };

} // namespace conan


#endif //_KEYVALUETABLEMODEL__27_11_09__15_20_44__H_
