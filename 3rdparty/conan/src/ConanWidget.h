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
    \brief Contains ConanWidget declaration
*/


#ifndef _CONANWIDGET__09_12_08__12_00_55__H_
#define _CONANWIDGET__09_12_08__12_00_55__H_


#include "ConanDefines.h"
#include "ui_ConanWidget.h"
#include <QtGui/QWidget>


class QSortFilterProxyModel;
class QStringListModel;
class QUndoStack;
class QXmlStreamWriter;


namespace conan {
    class ConnectionFilterProxyModel;
    class ConnectionModel;
    class KeyValueTableModel;
    class ObjectModel;
    class SignalSpyModel;
    struct ConnectionData;


    /*!
        \brief The main Conan widget that provides run-time signal/slot introspection.

        Object hierarchies can be added using \p AddRootObject or they can be discovered automatically using \p DiscoverObjects.

        The following shortcuts are supported:
            - Alt+Left - Go back to previous selected object
            - Alt+Right - Go forward to next selected object
            - Ctrl+F - Focus the find control to enter a search text
            - F3 - Find next matching object
            - F5 - Refresh all object hierarchies
            - Enter
                - When typing a search text - performs the actual search.
    */
    class CONAN_API ConanWidget : public QWidget {
        Q_OBJECT
        Q_CLASSINFO("Version", "1.0.2")
        Q_CLASSINFO("Author", "Elmar de Koning")
        Q_CLASSINFO("Contact", "edekoning@gmail.com")
        Q_CLASSINFO("Website", "http://sourceforge.net/projects/conanforqt")
        Q_CLASSINFO("License", "GPL")

    public:
        ConanWidget (QWidget* inParent = 0, Qt::WindowFlags inFlags = 0);

        QList <const QObject*> GetRootObjects () const;
        void AddRootObject (const QObject* inObject);
        void RemoveRootObject (const QObject* inObject);
        void RemoveAllRootObjects ();
        void DiscoverObjects ();

        void SetHeaderResizeMode (QHeaderView::ResizeMode mode);

    private:
        void InitObjectHierarchyTab ();
        void InitSignalSpiesTab ();
        void ClearCurrentObjectViews ();

        template <typename Pred>
        bool FindAndSelectObject (Pred inPred);
        bool BlockSelectionCommand (bool inBlock);

        void ExportToXML (QXmlStreamWriter& inWriter, const QModelIndex& inIndex) const;

    signals:
        //! \brief Subscribe to all signal spy log messages. Never ever connect a spy to this signal!!!
        void SignalSpyLog (const QString& msg);

    private slots:
        void SlotAbout ();
        void SlotCurrentObjectChanged (const QModelIndex& inCurrent, const QModelIndex& inPrevious);
        void SlotRefresh ();
        void SlotFindObject ();
        void SlotFindMethod (const QModelIndex& inProxyIndex);
        void SlotFindDuplicateConnection ();
        void SlotDiscoverObjects ();
        void SlotObjectContextMenuRequested (const QPoint& inPos);
        void SlotRemoveRootObject ();
        void SlotRemoveAllRootObjects ();
        void SlotConnectionContextMenuRequested (const QPoint& inPos);
        void SlotExportToXML ();

        void SlotSpiesContextMenuRequested (const QPoint& inPos);
        void SlotUpdateSignalLoggerOptions ();
        void SlotDeleteSpies ();

        void SlotRequestConfirmation (const QString& title, const QString& message, bool& confirmed);

    public:
        Ui::ConanWidget mForm;
        ObjectModel* mObjectModel;                      //!< The model containing the object hierarchies
        ConnectionModel* mSignalModel;                  //!< The model containing all signal connections for the current object
        ConnectionModel* mSlotModel;                    //!< The model containing all slot connections for the current object
        ConnectionFilterProxyModel* mProxySignalModel;  //!< Provides sorting and filtering for the signal model
        ConnectionFilterProxyModel* mProxySlotModel;    //!< Provides sorting and filtering for the slot model
        QStringListModel* mInheritanceModel;            //!< The model containing the inheritance data for the current object
        KeyValueTableModel* mClassInfoModel;            //!< The model containing the class info data for the current object
        SignalSpyModel* mSignalSpyModel;                //!< The model containing all signal spies
        QSortFilterProxyModel* mProxySignalSpyModel;    //!< Provides sorting for the signal spy model
        QUndoStack* mUndoStack;                         //!< Object selection undo stack
        bool mBlockSelectionCommand;                    //!< Used to prevent a selection command to generate a new selection command
    };

} // namespace conan


#endif //_CONANWIDGET__09_12_08__12_00_55__H_
