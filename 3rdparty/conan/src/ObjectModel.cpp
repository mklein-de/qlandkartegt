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
    \brief Contains ObjectModel related definitions
*/


#include "ObjectModel.h"
#include "ObjectUtility.h"
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QDesktopWidget>
#include <QtGui/QInputContext>
#include <QtGui/QStyle>
#include <QtGui/QWidget>


namespace conan {
    /*!
        \brief Constructs an item for the given object and all its children.
    */
    ObjectItem::ObjectItem (const QObject* inObject, const ObjectItem* inParent) :
        mParent (inParent), mObject (const_cast <QObject*> (inObject))
    {
        if (mObject) {
            // ensure we can stil display the item after its QObject has been destroyed
            mAddress = ObjectUtility::Address (mObject);
            mName = ObjectUtility::Name (mObject);
            mClass = ObjectUtility::Class (mObject);
            // recursively process all children
            foreach (const QObject* object, mObject->children ()) {
                mChildren.push_back (new ObjectItem (object, this));
            }
        }
    }

    /*!
        \brief Destructs the item and all its children.
    */
    ObjectItem::~ObjectItem () {
        Reset ();
    }

    /*!
        \brief Destroyes all children and resets all members.
    */
    void ObjectItem::Reset () {
        mObject = 0;
        mParent = 0;
        foreach (const ObjectItem* item, mChildren) {
            delete item;
        }
        mChildren.clear ();
    }

    /*!
        \brief Returns the index of this item at its parent.
    */
    int ObjectItem::Index () const {
        return mParent ? mParent->ChildIndex (this) : -1;
    }

    /*!
        \brief Returns the index of the given object if it is a child; otherwise -1.
    */
    int ObjectItem::ChildIndex (const ObjectItem* inChild) const {
        // perform a non-recursive search
        int index = qFind (mChildren.begin (), mChildren.end (), inChild) - mChildren.begin ();
        return index < static_cast <int> (mChildren.size ()) ? index : -1;
    }

    /*!
        \brief Returns the number of child items.
    */
    int ObjectItem::ChildCount () const {
        return static_cast <int> (mChildren.size ());
    }

    /*!
        \brief Returns the parent item.
    */
    const ObjectItem* ObjectItem::Parent () const {
        return mParent;
    }

    /*!
        \brief Returns the child item at the given index.
        Performs no range checking!
    */
    const ObjectItem* ObjectItem::Child (int inIndex) const {
        return mChildren [inIndex];
    }

    /*!
        \brief Returns the wrapped object
    */
    const QObject* ObjectItem::Object () const {
        return mObject;
    }

    /*!
        \brief Returns the object name of the wrapped object
    */
    const QString& ObjectItem::Name () const {
        return mName;
    }

    /*!
        \brief Returns the class name of the wrapped object
    */
    const QString& ObjectItem::Class () const {
        return mClass;
    }

    /*!
        \brief Returns the object address of the wrapped object
    */
    const QString& ObjectItem::Address () const {
        return mAddress;
    }


    // ------------------------------------------------------------------------------------------------


    /*!
        \brief Constructs an empty root item.
    */
    RootItem::RootItem () :
        ObjectItem ()
    {
    }

    /*!
        \brief Creates and adds an item hierarchy that wraps the object hierarchy defined by the given object.

        When the object is already part of an existing hierarchy nothing is done.
        In case the object shares a mutual parent with an existing hierarchy, that hierarchy is
        replaced with the hierarchy starting at that mutual parent.
    */
    void RootItem::AddChild (const QObject* inObject) {
        if (!inObject) {
            return;
        }
        // is the object known
        if (Find (MatchObjectByValue (ObjectUtility::Address (inObject)))) {
            return;
        }
        // is the object part of same object hierarchy as any of the existing children
        for (int c=0; c<mChildren.size (); c++) {
            if (const QObject* object = mChildren [c]->Object ()) {
                const QObject* parent = object->parent ();
                // check if the object is the direct parent of a child
                if (inObject != parent) {
                    parent = ObjectUtility::TopLevelParent (object);
                }
                // check if the object is toplevel parent of a child
                if (inObject != parent) {
                    parent = ObjectUtility::FindMutualParent (inObject, object);
                }
                // check if the object has a mutual parent with a child
                if (parent) {
                    // replace the child with the found parent
                    ObjectItem* item = mChildren [c];
                    delete item;
                    item = new ObjectItem (parent, this);
                    mChildren [c] = item;
                    return;
                }
            }
        }
        ObjectItem* item = new ObjectItem (inObject, this);
        mChildren.push_back (item);
    }

    /*!
      \brief Deletes and removes the item hierarchy that wraps the object hierarchy defined by the given object.
    */
    void RootItem::RemoveChild (const QObject* inObject) {
        for (QVector <ObjectItem*>::iterator it = mChildren.begin(); it != mChildren.end(); ++it) {
            ObjectItem* item = *it;
            if (item->Object() == inObject) {
                mChildren.erase(it);
                delete item;
                return;
            }
        }
    }

    /*!
        \brief Destroyes all child items.
    */
    void RootItem::RemoveAllChildren () {
        Reset (); // also clears mObject and mParent
    }


    // ------------------------------------------------------------------------------------------------


    ObjectModel::ObjectModel (QObject* inParent) :
        QAbstractItemModel (inParent),
        mRoot (0),
        mSortColumn (kObject),
        mSortOrder (Qt::AscendingOrder)
    {
        setObjectName ("ObjectModel");
        mRoot = new RootItem ();
        connect (this, SIGNAL (modelReset ()), this, SLOT (SlotSort ()));
    }


    /*!
        \brief Adds the object hierarchy of \p inObject to the model.
    */
    void ObjectModel::AddRootObject (const QObject* inObject) {
        mRoot->AddChild (inObject);
        reset ();
    }

    /*!
        \brief Removes the object hierarchy of \p inObject from the model.
    */
    void ObjectModel::RemoveRootObject (const QObject* inObject) {
        mRoot->RemoveChild (inObject);
        reset ();
    }

    /*!
        \brief Removes all the object hierarchies from the model.
    */
    void ObjectModel::RemoveAllRootObjects () {
        mRoot->RemoveAllChildren ();
        reset ();
    }

    /*!
        \brief Returns the number of rows under the given parent.
    */
    int ObjectModel::rowCount (const QModelIndex& inParent) const {
        if (const ObjectItem* item = GetItem (inParent)) {
            return item->ChildCount ();
        }
        return 0;
    }

    /*!
        \brief Returns the number of columns of the model.
    */
    int ObjectModel::columnCount (const QModelIndex& /*inParent*/) const {
        return kColumnCount;
    }

    /*!
        \brief Returns the item flags for the given index.
        Only items whose objects have not been detroyed are enabled and selectable.
    */
    Qt::ItemFlags ObjectModel::flags (const QModelIndex& inIndex) const {
        const ObjectItem* item = GetItem (inIndex);
        return item && item->Object () ? Qt::ItemIsEnabled | Qt::ItemIsSelectable : Qt::NoItemFlags;
    }

    /*!
        \brief Returns the data stored under the given role for the item referred to by the index.
    */
    QVariant ObjectModel::data (const QModelIndex& inIndex, int inRole) const {
        if (!mRoot || inRole != Qt::DisplayRole) {
            return QVariant ();
        }
        if (const ObjectItem* item = GetItem (inIndex)) {
            int column = inIndex.column ();
            if (column == kObject) {
                return ObjectUtility::QualifiedName (item->Class (), item->Name ());
            }
            else if (column == kAddress) {
                if (item->Object ()) {
                    return item->Address ();
                }
                else {
                    return item->Address () + " (destroyed)";
                }
            }
        }
        return QVariant ();
    }

    /*!
        \brief Returns the data for the given role and section in the header with the specified orientation.
    */
    QVariant ObjectModel::headerData (int inSection, Qt::Orientation inOrientation, int inRole) const {
        if (inRole != Qt::DisplayRole) {
            return QVariant ();
        }
        if (inOrientation == Qt::Horizontal) {
            if (inSection == kObject) {
                return QString ("Object");
            }
            else if (inSection == kAddress) {
                return QString ("Address");
            }
        }
        return QVariant ();
    }

    /*!
        \brief Returns the index of the item in the model specified by the given row, column and parent index.

        The internal pointer of an index is set to the corresponding parent ObjectItem.
    */
    QModelIndex ObjectModel::index (int inRow, int inColumn, const QModelIndex& inParent) const {
        if (hasIndex (inRow, inColumn, inParent)) {
            const ObjectItem* item = GetItem (inParent);
            if (item && inRow < item->ChildCount ()) {
                item = item->Child (inRow);
            }
            return createIndex (inRow, inColumn, const_cast <ObjectItem*> (item));
        }
        return QModelIndex ();
    }

    /*!
        \brief Returns the parent of the model item with the given index, or QModelIndex() if it has no parent.
    */
    QModelIndex ObjectModel::parent (const QModelIndex& inIndex) const {
        if (const ObjectItem* item = GetItem (inIndex)) {
            const ObjectItem* parent = item->Parent ();
            if (parent && parent != mRoot) {
                return createIndex (parent->Index (), 0, const_cast <ObjectItem*> (parent));
            }
        }
        return QModelIndex ();
    }

    /*!
        \brief Returns the item referred to by the given index.
    */
    const ObjectItem* ObjectModel::GetItem (const QModelIndex& inIndex) const {
        if (inIndex.isValid ()) {
            return reinterpret_cast <const ObjectItem*> (inIndex.internalPointer ());
        }
        return mRoot;
    }

    /*!
        \brief Returns the object wrapped by the item with the given index.
    */
    const QObject* ObjectModel::GetObject (const QModelIndex& inIndex) const {
        const ObjectItem* item = GetItem (inIndex);
        return item ? item->Object () : 0;
    }

    /*!
        \brief The complete object hierarchy is refreshed.
        Note the modelReset signal is emitted.
    */
    void ObjectModel::SlotRefresh () {
        // store current root objects
        QVector <QPointer <QObject> > rootObjects;
        int childCount = mRoot->ChildCount ();
        for (int c=0; c<childCount; c++) {
            const ObjectItem* item = mRoot->Child (c);
            if (QObject* object = const_cast <QObject*> (item->Object ())) {
                rootObjects.push_back (object);
            }
        }
        // destroy item hierarchy
        mRoot->RemoveAllChildren ();
        // restore previous root objects
        foreach (const QObject* object, rootObjects) {
            mRoot->AddChild (object);
        }
        reset ();
    }

    /*!
        \brief Sorts the model using the current sort settings \p mSortColumn and \p mSortOrder.
    */
    void ObjectModel::SlotSort () {
        emit layoutAboutToBeChanged ();
        // for each persistent index store the corresponding object address
        QModelIndexList persistentIndices = persistentIndexList ();
        QStringList addresses;
        foreach (QModelIndex index, persistentIndices) {
            const QObject* object = GetObject (index);
            addresses.append (ObjectUtility::Address (object));
        }
        // perform the actual sorting
        mRoot->Sort (ObjectItemSorter (mSortOrder, mSortColumn == kAddress));
        // update each persistent index
        foreach (QModelIndex from, persistentIndices) {
            int newRow = FindObject (MatchObjectByValue (addresses.front ())).row ();
            QModelIndex to = index (newRow, from.column (), from.parent ());
            changePersistentIndex (from, to);
            addresses.pop_front ();
        }
        emit layoutChanged ();
    }

    /*!
        \brief Finds all top level widgets and adds them to the model.
        Note the modelReset signal is emitted.
    */
    void ObjectModel::DiscoverRootObjects () {
        // discover top level parents of all current root objects
        QVector <QPointer <QObject> > rootObjects;
        int childCount = mRoot->ChildCount ();
        for (int c=0; c<childCount; c++) {
            if (const QObject* object = mRoot->Child (c)->Object ()) {
                const QObject* parent = ObjectUtility::TopLevelParent (object);
                rootObjects.push_back (const_cast <QObject*> (parent));
            }
        }
        // destroy item hierarchy
        mRoot->RemoveAllChildren ();
        // add the discovered top level parents
        foreach (const QObject* object, rootObjects) {
            mRoot->AddChild (object);
        }
        // add various static objects
        mRoot->AddChild (QCoreApplication::instance ());
        mRoot->AddChild (QApplication::desktop ());
        mRoot->AddChild (QApplication::clipboard ());
        mRoot->AddChild (QApplication::style ());

        // add all top level widgets retrieved from the application
        if (qApp) {
            QWidgetList widgets = qApp->topLevelWidgets ();
            foreach (const QWidget* widget, widgets) {
                mRoot->AddChild (widget);
            }
            mRoot->AddChild (qApp->inputContext ());
        }        
        reset ();
    }

    /*!
        \brief Returns the class name of the object wrapped by the item with the given index.
    */
    QString ObjectModel::GetClass (const QModelIndex& inIndex) const {
        if (const ObjectItem* item = GetItem (inIndex)) {
            return item->Class ();
        }
        return QString ();
    }

    /*!
        \brief Returns the object name of the object wrapped by the item with the given index.
    */
    QString ObjectModel::GetObjectName (const QModelIndex& inIndex) const {
        if (const ObjectItem* item = GetItem (inIndex)) {
            return item->Name ();
        }
        return QString ();
    }

    /*!
        \brief Returns the object address of the object wrapped by the item with the given index.
    */
    QString ObjectModel::GetAddress (const QModelIndex& inIndex) const {
        if (const ObjectItem* item = GetItem (inIndex)) {
            return item->Address ();
        }
        return QString ();
    }


    /*!
        \brief Sorts the model by the given column in the given order.
    */
    void ObjectModel::sort (int inColumn, Qt::SortOrder inOrder) {
        if (inColumn == mSortColumn && inOrder == mSortOrder) {
            return;
        }
        mSortColumn = inColumn;
        mSortOrder = inOrder;

        SlotSort ();
    }

} // namespace conan
