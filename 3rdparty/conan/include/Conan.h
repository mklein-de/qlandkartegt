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
    \brief Main include file for using Conan in other projects
*/


#include "../src/ConanWidget.h"
#include "../src/AboutDialog.h"



/*!
    \mainpage Conan - Connection Analyzer for Qt

    \image html Conan.jpg "The Conan widget 'conan::ConanWidget'"

    \section sec_conan Conan
    Conan is a C++ library that provides run-time introspection of object hierarchies, object
    inheritance, signal/slot connections, and signal emissions. \n

    Conan contains a single widget (conan::ConanWidget) that provides the
    following functionality:
    \li object hierarchies can be added, discovered, browsed and exported to XML
    \li signals/slots, including active connections, can be examined
    \li any signal/slot connection can be disconnected
    \li duplicate connections can be found
    \li signal emissions can be monitored and logged

    Conan has been used to identify multiple duplicate connections in the
    source code of Qt!

    Contact - edekoning@gmail.com \n
    Website - http://sourceforge.net/projects/conanforqt \n
    License - GPL (source code); CCL (icons) \n

    Other sites hosting Conan include:
    \li Qt-Apps.org - http://qt-apps.org/content/show.php/Conan+-+Connection+analyzer+for+Qt?content=108330
    \li Ohloh.net - http://www.ohloh.net/p/conanforqt
    \li FamousWhy.com - Granted an Editor's Pick Award - http://download.famouswhy.com/conan_for_qt/

    Other projects using Conan include:
    \li Qtilities - http://www.qtilities.org/

    <hr/><br/>

    \section sec_usage Usage
    \code
    #include <Conan.h>          // checks for proper qt version and includes
                                // the ConanWidget and AboutDialog

    //Q_INIT_RESOURCE (Conan);  // necessary on some platforms when Conan
                                // is build as a static library
    conan::ConanWidget widget;
    widget.AddRootObject (myMainWindow);    // optional
    widget.AddRootObject (someOtherObject); // optional
    widget.show ();
    \endcode

    <hr/><br/>

    \section sec_toolbar Toolbar
    \image html Toolbar.jpg "The Conan toolbar"
    \n
    From left to right:
    \li Back (Alt + Left) - Selects the previous object of the selection history in the \a Object \a Hierarchy tree view.
    \li Forward (Alt + Right) - Selects the next object of the selection history in the \a Object \a Hierarchy tree view.
    \li Find object (Ctrl + F) - Text field used to specify a search criteria that can be any part of an object's class
        name, object name, or address. Press Enter/Return to perform the actual search.
    \li Find Next (F3) - Finds and selects the next object that matches the current search criteria.
    \li Refresh objects (F5) - Refreshes the complete \a Object \a Hierarchy tree view.
    \li Discover objects - Discovers all top-level widgets and adds them to the \a Object \a Hierarchy tree view.
    \li Find duplicate connections - Scans the entire \a Object \a Hierarchy tree view for the next occurance of a
        duplicate connection. A duplicate connection occurs when a signal/slot is connected multiple times to the same
        signal/slot. Duplicate connections are marked by a bright yellow background. Note that duplicate connections
        may be hidden when the 'Hide inactive methods' or 'Hide inherited methods' option is enabled.
    \li Export to XML - Exports the selected \a QObject hierarchy, inlcuding signal, slots and all active connection
        to XML, e.g.:
        \code
<?xml version="1.0" encoding="UTF-8"?>
<?xml version="1.0" encoding="UTF-8"?>
<export conan="1.0.0" qt="4.7.0" created="2010-10-17T17:52:01">
    <object name="ConanWidget" class="conan::ConanWidget" address="0x0012fdc8">
        <Signal signature="customContextMenuRequested(QPoint)" access="Protected"
                declaredBy="QWidget"/>
        <Signal signature="destroyed()" access="Protected" declaredBy="QObject"/>
        <Signal signature="destroyed(QObject*)" access="Protected" declaredBy="QObject"/>
        <Slot signature="SlotRequestConfirmation(QString,QString,bool&amp;)" access="Private"
              declaredBy="conan::ConanWidget">
            <connection methodType="Signal" 
                        signature="SignalRequestConfirmation(QString,QString,bool&amp;)"
                        objectClass="conan::ConnectionModel" objectName="ConnectionModel"
                        objectAddress="0x00af24a8" connectionType="Auto" access="Protected"
                        declaredBy="conan::ConnectionModel"/>
            <connection methodType="Signal"
                        signature="SignalRequestConfirmation(QString,QString,bool&amp;)"
                        objectClass="conan::ConnectionModel" objectName="ConnectionModel"
                        objectAddress="0x00afef00" connectionType="Auto" access="Protected"
                        declaredBy="conan::ConnectionModel"/>
        </Slot>
        <Slot signature="SlotDeleteSpies()" access="Private" declaredBy="conan::ConanWidget">
            <connection methodType="Signal" signature="triggered(bool)" objectClass="QAction"
                        objectName="actionDeleteSpies" objectAddress="0x003ffcd8"
                        connectionType="Auto" access="Protected" declaredBy="QAction"/>
        </Slot>
        ...
        <object name="ConnectionModel" class="conan::ConnectionModel" address="0x00af24a8">
            <Signal signature="SignalRequestConfirmation(QString,QString,bool&amp;)"
                    access="Protected" declaredBy="conan::ConnectionModel">
                ...
            </Signal>
            ...
        </object>
        ...
    </object>
</export>
        \endcode
    \li About Conan - Shows the Conan about dialog.
        \image html About.jpg "The Conan about dialog"

    <hr/><br/>

    \section sec_object_hierarchy Object hierarchy
    \image html ObjectHierarchy.jpg "The Conan object hierarchy tree view"
    \n
    The \a Object \a Hierarchy tree view displayes the complete object hierarchy of one or more root objects. For each
    object, it displays the class name, object name, and object address. Note that this view is static in that it
    presents a snapshot in time of these hierarchies. The \a Refresh button from the toolbar can be used to synchronize
    the view with the current situation.

    Root objects can be added manually through \a ConanWidget::AddRootObject or automatically by using the \a Discover
    \a objects toolbar button. The \a Object \a Hierarchy tree view will make sure that each root object belongs to an
    unique object hierarchy. This is achieved by looking if the 'to be added' root object has a mutual parent with an
    existing root object. If so, the existing root object is replaced by this mutual parent. F.e: Object A has a child
    object B and a child object C. If we first add B and then add C, B will be replaced by A (the mutual parent of B
    and C).

    Root object can be removed through use of the context menu:
    \li Remove root object - Removes the current selected root object.
    \li Remove all root objects - Removes all root object.
    \li Refresh objects - Refreshes the complete \a Object \a Hierarchy tree view.
    \li Discover objects - Discovers all top-level widgets and adds them to the \a Object \a Hierarchy tree view.
    \li Find duplicate connections - Scans the entire \a Object \a Hierarchy tree view for the next occurance of a
        duplicate connection.
    \li Export to XML - Exports the selected \a QObject hierarchy, inlcuding signal, slots and all active connection
        to XML.

    Note that removing a child (non-root) object is not allowed, as that same child object would reappear after pressing \a Refresh
    from the toolbar.

    <hr/><br/>

    \section sec_inheritance Class inheritance
    \image html Inheritance.jpg "The Conan class inheritance list"
    \n
    The \a Class \a Inheritance list shows how the current selected object in the \a Object \a Hierarchy tree view was
    derived from QObject. It does not present a complete class inheritance hierarchy, but merely an inheritance list
    from QObject to the actual class of the selected object.

    <hr/><br/>

    \section sec_class_info Class information
    \image html ClassInformation.jpg "The Conan class information table"
    \n
    The \a Class \a Information table contains the extra class information of the current selected object. This meta
    data consists of key-value pairs that have been associated to the class using the Q_CLASSINFO macro.

    <hr/><br/>

    \section sec_signals_slots Signals and Slots
    \image html SignalsSlots.jpg "The Conan signal and slot views"
    \n
    All signals of the current selected object are listed in the \a Signals view, and all slots of the current selected
    object are listed in the \a Slots view. Both views are static and can be updated using the \a Refresh button from
    the toolbar. For each signal/slot the following information is provided:
    \li Access - The access level of the signal/slot: private (lock icon), protected (key icon), public (no icon).
    \li Signature - The normalized signature of the signal/slot; see QMetaObject::normalizedSignature.
    \li Declared in - The (sub)class in which the signal/slot was declared.

    All methods that are connected to a signal or slot, are displayed as children of that particular signal or slot.
    For each connection the following information is provided:
    \li Signature - The normalized signature of the connected signal/slot; see QMetaObject::normalizedSignature.
    \li Object - The object to which the connected signal/slot belongs; class name and object name.
    \li Address - The address of the object to which the connected signal/slot belongs.
    \li Connection - The type of connection (direct, queued, blocking, auto); see Qt::ConnectionType.

    Note that only connections between methods marked in code as signal or slot and processed by MOC are displayed.
    The conan::SignalSpy and QSignalSpy are examples of classes that have not been processed by MOC. They create
    connections directly by using the internal \a QMetaObject::connect. These types of connections are ignored.
    \n\n
    The \a Signals and \a Slots views can be filtered using the following options:
    \li Hide inactive methods - Hides all signals and slots that have no connections.
    \li Hide inherited methods - Hides all signals and slots that have been inherited.

    A context menu with the following options is provided for both the \a Signals and the \a Slots view:
    \li Disconnect all - Disconnects all receivers from the selected signal, or all senders from the selected slot.
    \li Disconnect - Disconnects the selected connection.
    \li Find object - Finds the object of the selected method in the \a Object \a Hierarchy tree view, selects it and
    expands the corresponding method in either the \a Signals or \a Slots view. Note that if the object is not present
    in the \a Object \a Hierarchy tree view the user is asked if it should be added as a root object.
    \li Spy signal - Creates a signal spy for the selected method; disabled when the selected method is not a signal.

    <hr/><br/>

    \section sec_signal_spies Signal spies
    \image html SignalSpies.jpg "The Conan signal spy table"
    \n
    The \a Signal \a spies \a table lists all existing signal spies. A signal spy monitors all emissions of a single
    signal. Each time it detects a signal emission, infomration about that signal emission is logged. For each signal
    spy the following information is provided:
    \li Signature - The normalized signature of the signal; see QMetaObject::normalizedSignature.
    \li Object - The object to which the signal belongs; class name and object name.
    \li Address - The address of the object to which the signal belongs.
    \li Declared in - The QObject subclass that declares the signal.

    Existing signal spies can be removed through use of the context menu:
    \li Select all (Ctrl + A) - Selects all available signal spies.
    \li Delete (Del) - Deletes all selected signal spies.

    <hr/><br/>

    \section sec_log_options Log options
    \image html LogOptions.jpg "The Conan signal spy log options"
    \n
    The \a log \a options define what information is logged by each signal spy and how each log is formatted. Currently
    all logging is performed through use of the \a qDebug function call. Two example logs are provided in the
    conan::ConanWidget that reflect the current options.
    \n\n
    When the \a Arguments option is enabled, each argument used during a signal emission is converted to a string. If
    this is not possible, the type name of the argument's type is used instead. In case no type name is associated \a
    void is used. Type names can be registered using qRegisterMetaType.

    \image html Output.jpg "Conan signal spy output"
*/
