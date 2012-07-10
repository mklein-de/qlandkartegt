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
    \brief Contains ConnectionModel related definitions
*/


#include "ConnectionModel.h"
#include "ObjectUtility.h"
#include "WaitCursor.h"
#include <QtGui/QApplication>
#include <QtGui/QBrush>
#include <QtGui/QFont>
#include <QtXml/QXmlStreamWriter>


namespace conan {

    /*!
        \brief Converts the given method access to a string.
    */
    QString MethodAccessToString (QMetaMethod::Access inAccess) {
        switch (inAccess) {
            case QMetaMethod::Protected:
                return "Protected";
            case QMetaMethod::Private:
                return "Private";
            case QMetaMethod::Public:
                return "Public";
            default:
                return "Unknown";
        }
    }

    /*!
        \brief Converts the given method type to a string.
    */
    QString MethodTypeToString (QMetaMethod::MethodType inType) {
        switch (inType) {
            case QMetaMethod::Method:
                return "Method";
            case QMetaMethod::Signal:
                return "Signal";
            case QMetaMethod::Slot:
                return "Slot";
            case QMetaMethod::Constructor:
                return "Constructor";
            default:
                return "Unknown";
        }
    }

    /*!
        \brief Converts the given connection type access to a string.
    */
    QString ConnectionTypeToString (uint inConnectionType) {
        switch (inConnectionType) {
            case Qt::DirectConnection:
                return "Direct";
            case Qt::QueuedConnection:
                return "Queued";
            case Qt::BlockingQueuedConnection:
                return "Blocking";
            case Qt::AutoConnection:
                return "Auto";
            case Qt::UniqueConnection:
                return "Unique";
            case Qt::AutoCompatConnection:
                return "AutoCompat";
            default:
                return "Unknown";
        }
    }


    // ------------------------------------------------------------------------------------------------


    MethodData::MethodData () :
        mObject (0),
        mConnectionType (Qt::AutoConnection),
        mAccess (QMetaMethod::Public)
    {}

    bool MethodData::operator!= (const MethodData& inSource) const {
        return mObject != inSource.mObject ||
               mSignature != inSource.mSignature ||
               mAddress != inSource.mAddress ||
               mName != inSource.mName ||
               mClass != inSource.mClass ||
               mSuperClass != inSource.mSuperClass ||
               mConnectionType != inSource.mConnectionType ||
               mAccess != inSource.mAccess ||
               mMethodType != inSource.mMethodType;
    }

    bool MethodData::operator== (const MethodData& inSource) const {
        return !(*this != inSource);
    }


    // ------------------------------------------------------------------------------------------------

    ConnectionData::ConnectionData ()
    {}

    ConnectionData::ConnectionData (const MethodData& inMethod) :
        mMethod (inMethod)
    {}

    bool ConnectionData::ContainsDuplicateConnections () const {
        if (mConnections.size () < 2) {
            return false;
        }
        foreach (const MethodData& data, mConnections) {
            if (1 < CountConnections (data)) {
                return true;
            }
        }
        return false;
    }

    int ConnectionData::CountConnections (const MethodData& inMethod) const {
        int count = 0;
        qCount (mConnections, inMethod, count);
        return count;
    }

    /*!
        \brief Exports the connection data to XML.
    */
    void ConnectionData::ExportToXML (QXmlStreamWriter& inWriter) const {
        inWriter.writeStartElement (MethodTypeToString(mMethod.mMethodType));
        inWriter.writeAttribute ("signature", mMethod.mSignature);
        inWriter.writeAttribute ("access", MethodAccessToString(mMethod.mAccess));
        inWriter.writeAttribute ("declaredBy", mMethod.mSuperClass);

        size_t connectionCount = mConnections.size ();
        for (size_t c=0; c<connectionCount; c++) {
            const MethodData& connection = mConnections [c];
            inWriter.writeStartElement ("connection");
            inWriter.writeAttribute ("methodType", MethodTypeToString(connection.mMethodType));
            inWriter.writeAttribute ("signature", connection.mSignature);
            inWriter.writeAttribute ("objectClass", connection.mClass);
            inWriter.writeAttribute ("objectName", connection.mName);
            inWriter.writeAttribute ("objectAddress", connection.mAddress);
            inWriter.writeAttribute ("connectionType", ConnectionTypeToString(connection.mConnectionType));
            inWriter.writeAttribute ("access", MethodAccessToString(connection.mAccess));
            inWriter.writeAttribute ("declaredBy", connection.mSuperClass);
            inWriter.writeEndElement ();
        }
        inWriter.writeEndElement ();
    }


    // ------------------------------------------------------------------------------------------------


    ConnectionModel::ConnectionModel (QObject* inParent) : QAbstractItemModel (inParent) {
        setObjectName ("ConnectionModel");
    }

    /*!
        \brief Resets the model with new source data.
    */
    void ConnectionModel::SetData (const QVector <ConnectionData>& inConnections) {
        mConnections = inConnections;
        reset ();
    }

    /*!
        \brief Disconnects the specified method.
        If the index corresponds to ConnectionData, al its children will be diconnected.
        If the index corresponds to MethodData, it will be disconnected from its parent.
    */
    void ConnectionModel::Disconnect (const QModelIndex& inIndex) {
        if (!inIndex.isValid ()) {
            return;
        }
        else if (const ConnectionData* connectionData = GetConnectionData (inIndex)) {
            // disconnect all recievers from current signal
            if (connectionData->mMethod.mMethodType == QMetaMethod::Signal) {
                bool confirmed = true;
                emit SignalRequestConfirmation (
                    "Disconnect", 
                    QString("Are you sure you want to disconnect" 
                            "\n\n<sender>   %1\n<signal>     %2"
                            "\n\nfrom all receivers?")
                            .arg (conan::ObjectUtility::QualifiedName(connectionData->mMethod.mObject))
                            .arg (connectionData->mMethod.mSignature),
                    confirmed);

                if (confirmed) {
                    WaitCursor wc;
                    beginRemoveRows (inIndex, 0, connectionData->mConnections.size () - 1);
                    // disconnect
                    disconnect (connectionData->mMethod.mObject,
                                TO_SIGNAL(connectionData->mMethod.mSignature),
                                0, 0);
                    // update model
                    mConnections [inIndex.row ()].mConnections.clear ();
                    endRemoveRows ();
                }
            }
            else if (connectionData->mMethod.mMethodType == QMetaMethod::Slot) {
                // disconnect all signals from current slot
                bool confirmed = true;
                emit SignalRequestConfirmation (
                    "Disconnect", 
                    QString("Are you sure you want to disconnect" 
                            "\n\n<receiver>   %1\n<method>    %2"
                            "\n\nfrom all senders?")
                            .arg (conan::ObjectUtility::QualifiedName(connectionData->mMethod.mObject))
                            .arg (connectionData->mMethod.mSignature),
                    confirmed);

                if (confirmed) {
                    WaitCursor wc;
                    beginRemoveRows (inIndex, 0, connectionData->mConnections.size () - 1);
                    // disconnect
                    foreach (const MethodData& methodData, connectionData->mConnections) {
                        disconnect (methodData.mObject,
                                    TO_SIGNAL(methodData.mSignature),
                                    connectionData->mMethod.mObject,
                                    TO_SLOT (connectionData->mMethod.mSignature));
                    }
                    // update model
                    mConnections [inIndex.row ()].mConnections.clear ();
                    endRemoveRows ();
                }
            }
        }
        // disconnect a single method
        else if (const MethodData* methodData = GetMethodData (inIndex)) {
            QModelIndex parent = inIndex.parent ();
            const ConnectionData* connectionData = GetConnectionData (parent);
            if (!connectionData) {
                return;
            }
            const MethodData* senderData = 0;
            const MethodData* receiverData = 0;

            // determine sender and receiver
            if (methodData->mMethodType == QMetaMethod::Signal) {
                senderData = methodData;
                receiverData = &connectionData->mMethod;
            }
            else if (methodData->mMethodType == QMetaMethod::Slot) {
                senderData = &connectionData->mMethod;
                receiverData = methodData;
            }
            bool confirmed = true;
                emit SignalRequestConfirmation (
                    "Disconnect", 
                    QString("Are you sure you want to disconnect" 
                            "\n\n<sender>   %1\n<signal>     %2"
                            "\n\nfrom"
                            "\n\n<receiver>   %3\n<method>    %4")
                            .arg (conan::ObjectUtility::QualifiedName(senderData->mObject))
                            .arg (senderData->mSignature)
                            .arg (conan::ObjectUtility::QualifiedName(receiverData->mObject))
                            .arg (receiverData->mSignature),
                    confirmed);

            if (confirmed) {
                WaitCursor wc;
                beginRemoveRows (parent, inIndex.row (), inIndex.row ());
                // disconnect
                disconnect (senderData->mObject,
                            TO_SIGNAL(senderData->mSignature),
                            receiverData->mObject,
                            receiverData->mMethodType == QMetaMethod::Slot
                                ? TO_SLOT(receiverData->mSignature)
                                : TO_SIGNAL(receiverData->mSignature));
                                
                // update model
                mConnections [parent.row ()].mConnections.remove (inIndex.row ());
                endRemoveRows ();
            }
        }
    }

    /*!
        \brief Returns the number of rows under the given parent.
    */
    int ConnectionModel::rowCount (const QModelIndex& inParent) const {
        if (!inParent.isValid ()) {
            return static_cast <int> (mConnections.size ());
        }
        else if (const ConnectionData* connectionData = GetConnectionData (inParent)) {
            return static_cast <int> (connectionData->mConnections.size ());
        }
        return 0;
    }


    /*!
        \brief Returns the number of columns of the model.
    */
    int ConnectionModel::columnCount (const QModelIndex& /*inParent*/) const {
        return kColumnCount;
    }

    /*!
        \brief Returns the item flags for the given index.
    */
    Qt::ItemFlags ConnectionModel::flags (const QModelIndex& inIndex) const {
        Qt::ItemFlags flags = Qt::ItemIsEnabled;
        // the access column only contains icons and is not selectable
        if (inIndex.column () != kAccess) flags |= Qt::ItemIsSelectable;
        return flags;
    }

    /*!
        \brief Returns the data stored under the given role for the item referred to by the index.
    */
    QVariant ConnectionModel::data (const QModelIndex& inIndex, int inRole) const {
        if (!inIndex.isValid () || mConnections.empty ()) {
            return QVariant ();
        }
        int column = inIndex.column ();
        if (inRole == Qt::DisplayRole || inRole == Qt::EditRole) {
            if (column == kSignature) {
                if (const MethodData* methodData = GetMethodData (inIndex)) {
                    return methodData->mSignature;
                }
                else if (const ConnectionData* connectionData = GetConnectionData (inIndex)) {
                    return connectionData->mMethod.mSignature;
                }
            }
            else if (column == kObject) {
                if (const MethodData* methodData = GetMethodData (inIndex)) {
                    return ObjectUtility::QualifiedName (methodData->mClass, methodData->mName);
                }
            }
            else if (column == kAddress) {
                if (const MethodData* methodData = GetMethodData (inIndex)) {
                    return methodData->mAddress;
                }
            }
            else if (column == kSuperClass) {
                if (const ConnectionData* connectionData = GetConnectionData (inIndex)) {
                    return connectionData->mMethod.mSuperClass;
                }
            }
            else if (column == kConnectionType) {
                if (const MethodData* methodData = GetMethodData (inIndex)) {
                    return ConnectionTypeToString (methodData->mConnectionType);
                }
            }
        }
        else if (inRole == Qt::BackgroundRole) {
            if (column != kAccess) {
                if (ContainsDuplicateConnections (inIndex)) {
                    return QBrush (Qt::yellow);
                }
            }
        }
        else if (inRole == Qt::ToolTipRole) {
            const MethodData* methodData = GetMethodData (inIndex);
            const ConnectionData* connectionData = GetConnectionData (inIndex);

            if (column == kAccess) {
                QMetaMethod::Access access = QMetaMethod::Public;
                if (methodData && GetConnectionData (inIndex.parent ())) {
                    access = methodData->mAccess;
                }
                else if (connectionData) {
                    access = connectionData->mMethod.mAccess;
                }
                return MethodAccessToString (access);
            }
            else if (ContainsDuplicateConnections (inIndex)) {
                return QString ("Duplicate connections");
            }
            else {
                if (methodData) {
                    return methodData->mSignature;
                }
                if (connectionData) {
                    return connectionData->mMethod.mSignature;
                }
            }
        }
        else if (inRole == Qt::DecorationRole) {
            if (column == kAccess) {
                QMetaMethod::Access access = QMetaMethod::Public;
                if (const ConnectionData* connectionData = GetConnectionData (inIndex)) {
                    access = connectionData->mMethod.mAccess;
                }
                if (access == QMetaMethod::Protected) {
                    return QPixmap (":/icons/conan/key");
                }
                else if (access == QMetaMethod::Private) {
                    return QPixmap (":/icons/conan/lock");
                }
            }
        }
        return QVariant ();
    }

    /*!
        \brief Returns the data for the given role and section in the header with the specified orientation.
    */
    QVariant ConnectionModel::headerData (int inSection, Qt::Orientation inOrientation, int inRole) const {
        if (inRole != Qt::DisplayRole) {
            return QVariant ();
        }
        if (inOrientation == Qt::Horizontal) {
            switch (inSection) {
                case kSignature:
                    return QString ("Signature");
                case kObject:
                    return QString ("Object");
                case kAddress:
                    return QString ("Address");
                case kSuperClass:
                    return QString ("Declared in");
                case kConnectionType:
                    return QString ("Connection");
                case kAccess:
                    return QString ("");
            }
        }
        return QVariant();
    }

    /*!
        \brief Returns the index of the item in the model specified by the given row, column and parent index.

        The internal pointer of an index is set to zero for indices that refer to top level signals/slots.
        For connected methods the internal pointer is set to the ConnectionData that contains them.
    */
    QModelIndex ConnectionModel::index (int inRow, int inColumn, const QModelIndex& inParent) const {
        if (hasIndex (inRow, inColumn, inParent)) {
            if (!inParent.isValid ()) {
                return createIndex (inRow, inColumn, 0);
            }
            else {
                return createIndex (inRow, inColumn, const_cast <ConnectionData*> (&mConnections [inParent.row ()]));
            }
        }
        return QModelIndex ();
    }

    /*!
        \brief Returns the parent of the model item with the given index, or QModelIndex() if it has no parent.

        Only items referring to connected methods have a parent.
    */
    QModelIndex ConnectionModel::parent (const QModelIndex& inIndex) const {
        if (!inIndex.internalPointer ()) {    // connection data
            return QModelIndex ();
        }
        int row = Row (reinterpret_cast <const ConnectionData*> (inIndex.internalPointer ()));
        return createIndex (row, 0, 0);
    }

    /*!
        \brief Retrieves the ConnectionData associated with the given index.
        \param[in] inIndex  The model index
        \return             The corresponding ConnectionData when \p index has no parent; otherwise 0
    */
    const ConnectionData* ConnectionModel::GetConnectionData (const QModelIndex& inIndex) const {
        if (inIndex.isValid () && !inIndex.parent ().isValid ()) {
            return reinterpret_cast <const ConnectionData*> (&mConnections [inIndex.row ()]);
        }
        return 0;
    }

    /*!
        \brief Retrieves the MethodData associated with the given index.
        \param[in] inIndex  The model index
        \return             The corresponding MethodData when \p index has a valid parent; otherwise 0
    */
    const MethodData* ConnectionModel::GetMethodData (const QModelIndex& inIndex) const {
        if (const ConnectionData* connectionData = reinterpret_cast <const ConnectionData*> (inIndex.internalPointer ())) {
            return &connectionData->mConnections [inIndex.row ()];
        }
        return 0;
    }

    /*!
        \brief Determines if the given index corresponds to a signal.
        \param[in] inIndex  The model index
        \return             Returns true when the given index corresponds to a singal; otherwise false.
    */
    bool ConnectionModel::IsSignal (const QModelIndex& inIndex) const {
        if (const ConnectionData* connectionData = GetConnectionData (inIndex)) {
            return connectionData->mMethod.mMethodType == QMetaMethod::Signal;
        }
        if (GetConnectionData (inIndex.parent ())) {
            if (const MethodData* methodData = GetMethodData (inIndex)) {
                return methodData->mMethodType == QMetaMethod::Signal;
            }
        }
        return false;
    }

    /*!
        \brief Determines if the ConnectionData, corresponding to the given index or its parent, contains duplicate connections.
        \param[in] inIndex  The model index
        \return             Returns true when there are duplicate connections; otherwise false.
    */
    bool ConnectionModel::ContainsDuplicateConnections (const QModelIndex& inIndex) const {
        if (const ConnectionData* connectionData = GetConnectionData (inIndex)) {
            return connectionData->ContainsDuplicateConnections ();
        }
        if (const ConnectionData* connectionData = GetConnectionData (inIndex.parent ())) {
            if (const MethodData* methodData = GetMethodData (inIndex)) {
                return 1 < connectionData->CountConnections (*methodData);
            }
        }
        return false;
    }

    /*!
        \brief Returns the row of the signal/slot that corresponds to the given ConnectionData
        \param[in] inConnectionData     The connection data
        \return                         The row of the signal/slot that corresponds to the connection data; otherwise -1
    */
    int ConnectionModel::Row (const ConnectionData* inConnectionData) const {
        if (inConnectionData) {
            for (int i=0; i<mConnections.size (); i++) {
                if (&mConnections [i] == inConnectionData) {
                    return static_cast <int> (i);
                }
            }
        }
        return -1;
    }


    // ------------------------------------------------------------------------------------------------


    ConnectionFilterProxyModel::ConnectionFilterProxyModel (QObject* inParent) :
        QSortFilterProxyModel (inParent), mInactiveFiltering (false), mInheritedFiltering (false)
    {
    }


    /*!
        \brief Sets the class name used during 'inherited method' filtering.
    */
    void ConnectionFilterProxyModel::SetClassName (const QString& inClassName) {
        mClassName = inClassName;
        invalidateFilter ();
    }

    /*!
        \brief Returns true if the item indicated by the given source row and parent should be included in the model; otherwise returns false.
    */
    bool ConnectionFilterProxyModel::filterAcceptsRow (int inSourceRow, const QModelIndex &inSourceParent) const {
        if (const ConnectionModel* model = dynamic_cast <ConnectionModel*> (sourceModel ())) {
            QModelIndex index = model->index (inSourceRow, 0, inSourceParent);
            if (model->GetMethodData (index)) {
                // never filter methods
                return true;
            }
            if (mInactiveFiltering && !model->rowCount (index)) {
                // filter connections without (connected) methods
                return false;
            }
            if (mInheritedFiltering) {
                QModelIndex superClassIndex = model->index (inSourceRow, ConnectionModel::kSuperClass, inSourceParent);
                if (model->data (superClassIndex, Qt::DisplayRole).toString () != mClassName) {
                    // filter connections declared in wrong classes
                    return false;
                }
            }
        }
        return true;
    }


    /*!
        \brief Returns true if the value of the item referred to by the left index is less than that of the right index, otherwise returns false.

        The column sort order is always: the primary (selected) sort column -> signature column -> object column -> address column.
    */
    bool ConnectionFilterProxyModel::lessThan (const QModelIndex& inLeft, const QModelIndex& inRight) const {
        if (inLeft.column () == ConnectionModel::kAccess) {
            // sortorder: access, signature, object, address
            if (const ConnectionModel* model = dynamic_cast <ConnectionModel*> (sourceModel ())) {
                QMetaMethod::Access leftAccess = QMetaMethod::Public;
                QMetaMethod::Access rightAccess = QMetaMethod::Public;
                if (const ConnectionData* leftConnection = model->GetConnectionData (inLeft)) {
                    leftAccess = leftConnection->mMethod.mAccess;
                }
                else if (const MethodData* leftMethod = model->GetMethodData (inLeft)) {
                    leftAccess = leftMethod->mAccess;
                }
                if (const ConnectionData* rightConnection = model->GetConnectionData (inRight)) {
                    rightAccess = rightConnection->mMethod.mAccess;
                }
                else if (const MethodData* rightMethod = model->GetMethodData (inRight)) {
                    rightAccess = rightMethod->mAccess;
                }
                if (leftAccess == rightAccess) {
                    QList <int> sortOrder;
                    sortOrder << ConnectionModel::kSignature << ConnectionModel::kObject << ConnectionModel::kAddress;
                    return lessThan (inLeft, inRight, sortOrder);
                }
                else {
                    // access values: private < protected < public
                    return leftAccess > rightAccess;
                }
            }
        }
        else if (inLeft.column () == ConnectionModel::kAddress) {
            // sortorder: address, signature, object
            QList <int> sortOrder;
            sortOrder << ConnectionModel::kAddress << ConnectionModel::kSignature << ConnectionModel::kObject;
            return lessThan (inLeft, inRight, sortOrder);
        }
        else if (inLeft.column () == ConnectionModel::kConnectionType) {
            // sortorder: connection type, signature, object, address
            QList <int> sortOrder;
            sortOrder << ConnectionModel::kConnectionType << ConnectionModel::kSignature << ConnectionModel::kObject << ConnectionModel::kAddress;
            return lessThan (inLeft, inRight, sortOrder);
        }
        else if (inLeft.column () == ConnectionModel::kObject) {
            // sortorder: object, signature, address
            QList <int> sortOrder;
            sortOrder << ConnectionModel::kObject << ConnectionModel::kSignature << ConnectionModel::kAddress;
            return lessThan (inLeft, inRight, sortOrder);
        }
        else if (inLeft.column () == ConnectionModel::kSignature) {
            // sortorder: signature, object, address
            QList <int> sortOrder;
            sortOrder << ConnectionModel::kSignature << ConnectionModel::kObject << ConnectionModel::kAddress;
            return lessThan (inLeft, inRight, sortOrder);
        }
        else if (inLeft.column () == ConnectionModel::kSuperClass) {
            // sortorder: superClass, signature, object, address
            QList <int> sortOrder;
            sortOrder << ConnectionModel::kSuperClass << ConnectionModel::kSignature << ConnectionModel::kObject << ConnectionModel::kAddress;
            return lessThan (inLeft, inRight, sortOrder);
        }
        return false;
    }

    /*!
        \brief Performs the actual value based index sorting using the given sort column order.
    */
    bool ConnectionFilterProxyModel::lessThan (const QModelIndex& inLeft, const QModelIndex& inRight, const QList <int>& inSortOrder) const {
        foreach (int column, inSortOrder) {
            QString leftData = sourceModel ()->data (inLeft.sibling (inLeft.row (), column)).toString ();
            QString rightData = sourceModel ()->data (inRight.sibling (inRight.row (), column)).toString ();
            if (leftData == rightData) {
                continue;
            }
            return leftData < rightData;
        }
        return false;
    }

    /*!
        Controls inactive method filtering
    */
    void ConnectionFilterProxyModel::SlotEnableInactiveFiltering (bool inEnable) {
        mInactiveFiltering = inEnable;
        invalidateFilter ();
    }

    /*!
        Controls inherited method filtering
    */
    void ConnectionFilterProxyModel::SlotEnableInheritedFiltering (bool inEnable) {
        mInheritedFiltering = inEnable;
        invalidateFilter ();
    }

} // namespace conan
