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
    \brief Contains KeyValueTableModel related definitions
*/


#include "KeyValueTableModel.h"


namespace conan {

    KeyValueTableModel::KeyValueTableModel (QObject* inParent) :
        QAbstractTableModel (inParent)
    {
        setObjectName ("KeyValueTableModel");
    }

    void KeyValueTableModel::SetClassInfo (const QMap <QString, QString>& inInfo) {
        mData = inInfo;
        reset ();
    }

    int KeyValueTableModel::rowCount (const QModelIndex& /*inParent*/) const {
        return mData.size ();
    }

    int KeyValueTableModel::columnCount (const QModelIndex& /*inParent*/) const {
        return 2;
    }

    QVariant KeyValueTableModel::data (const QModelIndex& inIndex, int inRole) const {
        if (!inIndex.isValid ()) {
            return QVariant ();
        }
        if (inRole == Qt::DisplayRole) {
            if (inIndex.column () == 0) {
                return (mData.begin () + inIndex.row ()).key () + "  -";
            }
            if (inIndex.column () == 1) {
                return (mData.begin () + inIndex.row ()).value ();
            }
        }
        else if (inRole == Qt::TextAlignmentRole) {
            if (inIndex.column () == 0) {
                return Qt::AlignRight;
            }
            if (inIndex.column () == 1) {
                return Qt::AlignLeft;
            }
        }
        return QVariant ();
    }

} // namespace conan
