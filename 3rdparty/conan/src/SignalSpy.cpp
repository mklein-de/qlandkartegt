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
    \brief Contains SignalSpy related definitions
*/


#include "ObjectUtility.h"
#include "SignalSpy.h"
#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QList>
#include <QtCore/QMetaMethod>
#include <QtCore/QMetaObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QtDebug>
#include <QtCore/QVariant>
#include <QtGui/QHeaderView>
#include <QtGui/QKeyEvent>


namespace conan {

    namespace /*unnamed*/ {

        /*!
            \brief Returns true when the given signal is monitored by the given spy.
        */
        bool SameSignal (const QObject* inObject, const QString& inSignal, const SignalSpy* inSpy) {
            if (inSpy->SignalData ().mObject) {
                return inSpy->SignalData ().mObject == inObject &&
                       inSpy->SignalData ().mSignature == inSignal;
            }
            QString address = ObjectUtility::Address (inObject);
            QString className = ObjectUtility::Class (inObject);
            QString objectName = ObjectUtility::Name (inObject);

            return inSpy->SignalData ().mSignature == inSignal &&
                   inSpy->SignalData ().mAddress == address &&
                   inSpy->SignalData ().mClass == className &&
                   inSpy->SignalData ().mName == objectName;
        }
    }

    /*!
        \brief Creates a SignalSpy that outputs emissions of the given signal (normalized signature).

        All actual logging is delegated to the given logger.
    */
    SignalSpy::SignalSpy (const QObject* inObject, const QString& inSignal, SignalLogger* inLogger) :
        mLogger (inLogger),
        mEmitCount (0)
    {
        static const int sMemberOffset = QObject::staticMetaObject.methodCount ();

        Q_ASSERT (inObject && inLogger);

        const QMetaObject* metaObject = inObject->metaObject ();
        int sigIndex = metaObject->indexOfMethod (inSignal.toAscii ().data ());
        if (sigIndex < 0) {
            qWarning ("SignalSpy: No such signal: '%s'", inSignal.toAscii ().data ());
            return;
        }
        if (!QMetaObject::connect (inObject, sigIndex, this, sMemberOffset, Qt::DirectConnection, 0)) {
            qWarning ("SignalSpy: QMetaObject::connect returned false. Unable to connect.");
            return;
        }

        QMetaMethod method = metaObject->method (sigIndex);
        mSignalData.mAccess = method.access ();
        mSignalData.mAddress = ObjectUtility::Address (inObject);
        mSignalData.mObject = const_cast <QObject*> (inObject);
        mSignalData.mName = ObjectUtility::Name (inObject);
        mSignalData.mClass = ObjectUtility::Class (inObject);
        mSignalData.mSignature = method.signature ();
        mSignalData.mSuperClass = metaObject->className ();
        mSignalData.mMethodType = method.methodType ();

        InitArgs (metaObject->method (sigIndex));
    }

    /*!
        \brief Called whenever a signal emission occurs
    */
    int SignalSpy::qt_metacall (QMetaObject::Call inCall, int inMethodId, void **inArgs) {
        inMethodId = QObject::qt_metacall (inCall, inMethodId, inArgs);
        if (inMethodId < 0) {
            return inMethodId;
        }
        if (inCall == QMetaObject::InvokeMetaMethod) {
            if (inMethodId == 0) {
                ProcessArgs (inArgs);
            }
            --inMethodId;
        }
        return inMethodId;
    }

    /*!
        \brief Stores the argument types for the signal being monitored
    */
    void SignalSpy::InitArgs (const QMetaMethod& inMember) {
        QList <QByteArray> params = inMember.parameterTypes ();
        for (int i = 0; i < params.count (); ++i) {
            int tp = QMetaType::type (params.at (i).constData ());
            if (tp == QMetaType::Void)
                qWarning ("Don't know how to handle '%s', use qRegisterMetaType to register it.",
                         params.at (i).constData ());
            mArgTypes << tp;
        }
    }

    /*!
        \brief Processes the arguments of a signal emission and outputs information about that signal emission.
    */
    void SignalSpy::ProcessArgs (void **inArgs) {
        mEmitCount++;
        // build a string of all given arguments, use values of they can be converted to string otherwise use types
        QString argString;
        if (mArgTypes.count()) {
            argString.append (" {");
            QList <QVariant> list;
            for (int i = 0; i < mArgTypes.count (); ++i) {
                QMetaType::Type type = static_cast <QMetaType::Type>(mArgTypes.at( i));
                QVariant param = QVariant (type, inArgs [i + 1]);
                if (param.canConvert (QVariant::String)) {
                    argString.append (param.toString ());
                }
                else {
                    argString.append (QMetaType::typeName (type));
                }
                if (i != mArgTypes.count () - 1) {
                    argString.append (", ");
                }
            }
            argString.append ("}");
        }
        if (mLogger) {
            mLogger->Log (this, argString);
        }
    }

    /*!
        \brief Returns information about the signal being spied.
    */
    const MethodData& SignalSpy::SignalData () const {
        return mSignalData;
    }

    /*!
        \brief Returns the number of monitored emissions for the signal.
    */
    int SignalSpy::EmitCount () const {
        return mEmitCount;
    }


    // ------------------------------------------------------------------------------------------------


    /*!
        \brief Creates a SignalLogger that initially only logs the signal signature and arguments.
    */
    SignalLogger::SignalLogger () :
        mShowTimestamp (false),
        mShowObject (false),
        mShowAddress (false),
        mShowSignature (true),
        mShowEmitCount (false),
        mShowArguments (true),
        mObjectWidth (kDynamicWidth),
        mSignatureWidth (kDynamicWidth),
        mEmitCountWidth (4),
        mPrettyFormatting (false),
        mSeparator (',')
    {
    }

    /*!
        \brief Outputs a single signal emission for the given signal spy and signal arguments.
    */
    void SignalLogger::Log (SignalSpy* inSpy, const QString& inArgs) {
        QString logMsg = BuildLogMessage (inSpy->SignalData (), inSpy->EmitCount (), inArgs);
        qDebug () << logMsg;

        // prevent recursion when the SignalSpyLog signal is being spied
        if (inSpy->SignalData ().mObject != this) {
            emit SignalSpyLog (logMsg);
        }
    }

    /*!
        \brief Returns the log message created for the given signal emission data.
    */
    QString SignalLogger::BuildLogMessage (const MethodData& inSignalData, int inEmitCount, const QString& inArgs) const {
        QString logMsg;
        QString separator = QString (" %1 ").arg (mSeparator);
        if (mShowTimestamp) {
            QString timestamp = QDateTime::currentDateTime ().toString (Qt::ISODate);
            timestamp.replace ('T', ' ');
            logMsg.append (timestamp);
        }
        if (mShowObject) {
            QString object = ObjectUtility::QualifiedName (inSignalData.mClass, inSignalData.mName);
            if (mPrettyFormatting && mObjectWidth != kDynamicWidth) {
                object = object.leftJustified (mObjectWidth, ' ', true);
            }
            if (!logMsg.isEmpty ()) {
                logMsg.append (separator);
            }
            logMsg.append (object);
        }
        if (mShowAddress) {
            QString address = inSignalData.mAddress;
            if (!logMsg.isEmpty ()) {
                logMsg.append (separator);
            }
            logMsg.append (address);
        }
        if (mShowSignature) {
            QString signature = inSignalData.mSignature;
            if (mPrettyFormatting && mSignatureWidth != kDynamicWidth) {
                signature = signature.leftJustified (mSignatureWidth, ' ', true);
            }
            if (!logMsg.isEmpty ()) {
                logMsg.append (separator);
            }
            logMsg.append (signature);
        }
        if (mShowEmitCount) {
            QString emitCount = QString::number (inEmitCount);
            if (mPrettyFormatting && mEmitCountWidth != kDynamicWidth) {
                emitCount = emitCount.leftJustified (mEmitCountWidth, ' ', true);
            }
            if (!logMsg.isEmpty ()) {
                logMsg.append (separator);
            }
            logMsg.append ('#');
            logMsg.append (emitCount);
        }
        if (mShowArguments && !inArgs.isEmpty ()) {
            QString arguments = inArgs;
            if (!logMsg.isEmpty ()) {
                logMsg.append (separator);
            }
            logMsg.append (arguments);
        }
        return logMsg;
    }

    /*!
        \brief Returns an example signal spy log based on the current log options.
    */
    QString SignalLogger::GetExampleLog () const {
        MethodData dummySignalData1;
        dummySignalData1.mSignature = "MySignal1()";
        dummySignalData1.mAddress = "0x12345678";
        dummySignalData1.mName = "DummyObject";
        dummySignalData1.mClass = "MyObject";

        MethodData dummySignalData2;
        dummySignalData2.mSignature = "MySignal2(int,const QString&)";
        dummySignalData2.mAddress = "0x87654321";
        dummySignalData2.mName = "MyDummy";
        dummySignalData2.mClass = "MyObject";

        // we need to temporarily adjust the object and signature width;
        SignalLogger* logger = const_cast <SignalLogger*> (this);
        // store old
        unsigned oldObjectWidth = logger->mObjectWidth;
        unsigned oldSignatureWidth = logger->mSignatureWidth;
        // adjust
        logger->mObjectWidth = ObjectUtility::QualifiedName (dummySignalData1.mClass, dummySignalData1.mName).count ();
        logger->mSignatureWidth = dummySignalData2.mSignature.count ();

        QString logMsg1 = BuildLogMessage (dummySignalData1, 7, QString ());
        QString logMsg2 = BuildLogMessage (dummySignalData2, 12, "{42, question}");

        logMsg1.count ();
        logMsg2.count ();
        // restore
        logger->mObjectWidth = oldObjectWidth;
        logger->mSignatureWidth = oldSignatureWidth;

        return logMsg1 + '\n' + logMsg2;
    }


    // ------------------------------------------------------------------------------------------------


    SignalSpyModel::SignalSpyModel (QObject* inParent)
        : QAbstractTableModel (inParent) {
    }

    SignalSpyModel::~SignalSpyModel () {
        foreach (SignalSpy* spy, mSignalSpies) {
            spy->deleteLater ();
        }
        mSignalSpies.clear ();
    }

    SignalLogger& SignalSpyModel::GetLogger () {
        return mLogger;
    }

    /*!
        \brief Returns true when a signal spy exists that monitors the given signal (normalized signature).
    */
    bool SignalSpyModel::ContainsSignalSpy (const QObject* inObject, const QString& inSignal) const {
        if (inObject && !inSignal.isEmpty ()) {
            foreach (SignalSpy* spy, mSignalSpies) {
                if (SameSignal (inObject, inSignal, spy)) {
                    return true;
                }
            }
        }
        return false;
    }

    /*!
        \brief Creates a signal spy that monitors the given signal (normalized signature).
    */
    void SignalSpyModel::CreateSignalSpy (const QObject* inObject, const QString& inSignal) {
        if (ContainsSignalSpy (inObject, inSignal)) {
            return;
        }
        QScopedPointer <SignalSpy> spy (new SignalSpy (inObject, inSignal, &mLogger));
        if (spy->SignalData ().mObject) {
            mLogger.mObjectWidth = qMax <unsigned> (mLogger.mObjectWidth, ObjectUtility::QualifiedName (spy->SignalData ().mClass, spy->SignalData ().mName).count ());
            mLogger.mSignatureWidth = qMax <unsigned> (mLogger.mSignatureWidth, spy->SignalData ().mSignature.count ());

            beginInsertRows (QModelIndex (), rowCount (), rowCount ());
            mSignalSpies.push_back (spy.take ());
            endInsertRows ();
        }
    }

    /*!
        \brief Destroyes the signal spy that monitors the given signal (normalized signature).
    */
    void SignalSpyModel::DestroySignalSpy (const QObject* inObject, const QString& inSignal) {
        int row = 0;
        foreach (SignalSpy* spy, mSignalSpies) {
            if (SameSignal (inObject, inSignal, spy)) {
                removeRows (row, 1);
                break;
            }
            ++row;
        }
    }

    /*!
        \brief Returns the number of signal spies.
    */
    int SignalSpyModel::rowCount (const QModelIndex& /*inParent*/) const {
        return static_cast <int> (mSignalSpies.size ());
    }

    /*!
        \brief Returns the number of columns for the children of the given parent.
    */
    int SignalSpyModel::columnCount (const QModelIndex& /*inParent*/) const {
        return kColumnCount;
    }

    /*!
        \brief Returns the item flags for the spy referred to by the index
    */
    Qt::ItemFlags SignalSpyModel::flags (const QModelIndex& /*inIndex*/) const {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    /*!
        \brief Returns the data stored under the given role for the spy referred to by the index.
    */
    QVariant SignalSpyModel::data (const QModelIndex& inIndex, int inRole) const {
        if (inIndex.isValid () && inRole == Qt::DisplayRole) {
            SignalSpy* spy = mSignalSpies [inIndex.row ()];
            switch (inIndex.column ()) {
                case kSignature:
                    return spy->SignalData ().mSignature;
                case kObject:
                    return ObjectUtility::QualifiedName (spy->SignalData ().mClass, spy->SignalData ().mName);
                case kAddress:
                    if (spy->SignalData ().mObject) {
                        return spy->SignalData ().mAddress;
                    }
                    else {
                        return spy->SignalData ().mAddress + " (destroyed)";
                    }
                case kSuperClass:
                    return spy->SignalData ().mSuperClass;
                default:
                    return QVariant ();
            }
        }
        return QVariant ();
    }

    /*!
        \brief Returns the data for the given role and section in the header with the specified orientation.
    */
    QVariant SignalSpyModel::headerData (int inSection, Qt::Orientation inOrientation, int inRole) const {
        if (inOrientation == Qt::Horizontal && inRole == Qt::DisplayRole) {
            switch (inSection) {
                case kSignature:
                    return QString ("Signature");
                case kObject:
                    return QString ("Object");
                case kAddress:
                    return QString ("Address");
                case kSuperClass:
                    return QString ("Declared in");
                default:
                    return QVariant ();
            }
        }
        return QVariant ();
    }

    /*!
        \brief Removes count spies starting with the given row. Returns true if the spies were successfully removed; otherwise returns false.
    */
    bool SignalSpyModel::removeRows (int inRow, int inCount, const QModelIndex& inParent) {
        if (!hasIndex (inRow, 0, inParent) || inCount <= 0) {
            return false;
        }
        beginRemoveRows (QModelIndex (), inRow, inRow + inCount - 1);
        for (int r=0; r<inCount; r++) {
            mSignalSpies [inRow + r]->deleteLater ();
        }
        QVector <SignalSpy*>::iterator first = mSignalSpies.begin () + inRow;
        QVector <SignalSpy*>::iterator last = first + inCount;
        mSignalSpies.erase (first, last);
        endRemoveRows ();

        // update logger object and signature width options
        mLogger.mObjectWidth = SignalLogger::kDynamicWidth;
        mLogger.mSignatureWidth = SignalLogger::kDynamicWidth;

        foreach (SignalSpy* spy, mSignalSpies) {
            mLogger.mObjectWidth = qMax <unsigned> (mLogger.mObjectWidth, ObjectUtility::QualifiedName (spy->SignalData ().mClass, spy->SignalData ().mName).count ());
            mLogger.mSignatureWidth = qMax <unsigned> (mLogger.mSignatureWidth, spy->SignalData ().mSignature.count ());
        }
        return true;
    }
}


