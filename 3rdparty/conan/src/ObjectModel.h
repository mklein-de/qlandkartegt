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
    \brief Contains ObjectModel related declarations
*/


#ifndef _OBJECTMODEL__09_12_08__09_19_24__H_
#define _OBJECTMODEL__09_12_08__09_19_24__H_


#include "ConanDefines.h"
#include <QtCore/QAbstractItemModel>
#include <QtCore/QPointer>
#include <QtCore/QVector>
#include <QtCore/QtAlgorithms>


// prevent the preprocessor to change GetObject to GetObjectA or GetObjectW
#ifdef GetObject
#undef GetObject
#endif


namespace conan {

    class RootItem;


    /*!
        \brief Used to wrap QObjects for use in a ObjectModel.
    */
    class CONAN_LOCAL ObjectItem {

    public:
        ObjectItem (const QObject* inObject=0, const ObjectItem* inParent=0);
        virtual ~ObjectItem ();

        int Index () const;
        int ChildIndex (const ObjectItem* inChild) const;
        int ChildCount () const;

        const ObjectItem* Parent () const;
        const ObjectItem* Child (int inIndex) const;
        const QObject* Object () const;

        const QString& Name () const;
        const QString& Class () const;
        const QString& Address () const;

        template <typename Pred>
        const ObjectItem* Find (Pred inPred, const ObjectItem* inStartItem=0, bool* inActive=0) const;

        template <typename Pred>
        void Sort (Pred inPred);

    protected:
        void Reset ();

    protected:
        const ObjectItem* mParent;
        QVector <ObjectItem*> mChildren;
        QPointer <QObject> mObject;             //!< The wrapped QObject, or 0 when it has been destroyed
        QString mAddress;                       //!< The address of mObject
        QString mName;                          //!< The object name of mObject
        QString mClass;                         //!< The class name of mObject
    };

    /*!
        \brief Starts a recursive search for an item whose name, class or address matches a given value.
        \param[in] inPred       A unary predicate function object that defines the matching criterion
        \param[in] inStartItem  (optional) The item at which the search starts or stops; depends on \p inActive
        \param[in] inActive     (optional) When false the search starts at \p inStartItem; otherwise the search stops at \p inStartItem
        \return                 The item that matches \p inValue; otherwise 0
    */
    template <typename Pred>
    const ObjectItem* ObjectItem::Find (Pred inPred, const ObjectItem* inStartItem, bool* inActive) const {
        if (inActive && inStartItem == this) {
            // (de)active the actual matching when the startItem has been encountered
            (*inActive) = !(*inActive);
        }
        foreach (const ObjectItem* item, mChildren) {
            if (!inActive || (*inActive)) {
                if (inPred (item)) {
                    return item;
                }
            }
            // recurse
            if (const ObjectItem* child = item->Find (inPred, inStartItem, inActive)) {
                return child;
            }
        }
        return 0;
    }

    /*!
        \brief Sort all children recursively.
        \param[in] inPred   A binary predicate function object that defines the comparison criterion
    */
    template <typename Pred>
    void ObjectItem::Sort (Pred inPred) {
        qSort (mChildren.begin (), mChildren.end (), inPred);

        foreach (ObjectItem* item, mChildren) {
            item->Sort (inPred);
        }
    }


    // ------------------------------------------------------------------------------------------------


    /*!
        \brief Represents the root item for use in a ObjectModel.
        Note that the root does not wrap an object.
    */
    class CONAN_LOCAL RootItem : public ObjectItem {

    public:
        RootItem ();
        void AddChild (const QObject* inObject);
        void RemoveChild (const QObject* inObject);
        void RemoveAllChildren ();
    };


    // ------------------------------------------------------------------------------------------------


     /*!
        \brief A unary function object that defines a matching criteria for ObjectItem
        \param[in] inValue          Defines the value that is macthed to the class, name, address of an ObjectItem
    */
    struct CONAN_LOCAL MatchObjectByValue
    {
        MatchObjectByValue (const QString& inValue) :
            mValue (inValue)
        {}

        bool operator () (const ObjectItem* inItem) {
            return (inItem && (
                        inItem->Name ().contains (mValue, Qt::CaseInsensitive) ||
                        inItem->Class ().contains (mValue, Qt::CaseInsensitive) ||
                        inItem->Address ().contains (mValue, Qt::CaseInsensitive)));
        }

        QString mValue;
    };


    // ----------------------------------------------------------------------------------------


    /*!
        \brief A binary function object that defines a sorting criteria for ObjectItem: class, name, address
        \param[in] inOrder          Defines the sortorder, either ascending or descending
        \param[in] inAddressFirst   When true items are first sorted by address, then by class and name
    */
    struct CONAN_LOCAL ObjectItemSorter
    {
        ObjectItemSorter (Qt::SortOrder inOrder, bool inAddressFirst) :
            mAddressFirst (inAddressFirst),
            mSortOrder (inOrder)
        {}

        bool operator () (const ObjectItem* inItem1, const ObjectItem* inItem2) {
            if (mAddressFirst && inItem1->Address () != inItem2->Address ()) {
                return mSortOrder == Qt::AscendingOrder ?
                    inItem1->Address () < inItem2->Address () :
                inItem1->Address () > inItem2->Address ();
            }
            if (inItem1->Class () == inItem2->Class ()) {
                if (inItem1->Name () == inItem2->Name ()) {
                    return mSortOrder == Qt::AscendingOrder ?
                        inItem1->Address () < inItem2->Address () :
                    inItem1->Address () > inItem2->Address ();
                }
                else {
                    return mSortOrder == Qt::AscendingOrder ?
                        inItem1->Name () < inItem2->Name () :
                    inItem1->Name () > inItem2->Name ();
                }
            }
            else {
                return mSortOrder == Qt::AscendingOrder ?
                    inItem1->Class () < inItem2->Class () :
                inItem1->Class () > inItem2->Class ();
            }
        }

        bool mAddressFirst;
        Qt::SortOrder mSortOrder;
    };


    // --------------------------------------------------------------------------------------------


    /*!
        \brief A read-only model for displaying one or more object hierarchies.

        Note that this model is static. It does not update automatically when objects are created
        or destroyed. However, it does detect when objects are destroyed and marks the corresponding
        items disabled and non-selectable. The function SlotRefresh is provided to manually update
        all object hierarchies.
    */
    class CONAN_LOCAL ObjectModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        typedef enum COLUMNS {
            kObject,
            kAddress,
            kColumnCount
        } Columns;

    public:
        ObjectModel (QObject* inParent = 0);

        void AddRootObject (const QObject* inObject);
        void RemoveRootObject (const QObject* inObject);
        void RemoveAllRootObjects ();
        void DiscoverRootObjects ();

        template <typename Pred>
        QModelIndex FindObject (Pred inPred, const QModelIndex& inStart = QModelIndex ()) const;

        const QObject* GetObject (const QModelIndex& inIndex) const;
        QString GetClass (const QModelIndex& inIndex) const;
        QString GetObjectName (const QModelIndex& inIndex) const;
        QString GetAddress (const QModelIndex& inIndex) const;

        // qt overrides
        virtual int rowCount (const QModelIndex& inParent = QModelIndex ()) const;
        virtual int columnCount (const QModelIndex& inParent = QModelIndex ()) const;
        virtual Qt::ItemFlags flags (const QModelIndex& inIndex) const;
        virtual QVariant data (const QModelIndex& inIndex, int inRole) const;
        virtual QVariant headerData (int inSection, Qt::Orientation inOrientation, int inRole = Qt::DisplayRole) const;
        virtual QModelIndex index (int inRow, int inColumn, const QModelIndex& inParent = QModelIndex ()) const;
        virtual QModelIndex parent (const QModelIndex& inIndex) const;
        virtual void sort (int inColumn, Qt::SortOrder inOrder = Qt::AscendingOrder);

    private:
        const ObjectItem* GetItem (const QModelIndex& inIndex) const;

    public slots:
        void SlotRefresh ();

    private slots:
        void SlotSort ();

    private:
        RootItem* mRoot;            //!< The root item of the model that contains all object hierarchies
        int mSortColumn;            //!< The primary sort column
        Qt::SortOrder mSortOrder;   //!< The current sort order (ascending or descending)
    };

    /*!
        \brief Finds the object whose class, name or address matches the given value.
        \param[in] inPred   A unary predicate function object that defines the matching criterion
        \param[in] inStart  The item to start searching from (note that the search wraps)
        \return             The item that wraps the found object
    */
    template <typename Pred>
    QModelIndex ObjectModel::FindObject (Pred inPred, const QModelIndex& inStart) const {
        const ObjectItem* startItem = GetItem (inStart);
        bool active = false;
        // find starting from startItem
        const ObjectItem* result = mRoot->Find (inPred, startItem, &active);
        if (!result) {
            // wrap search
            active = true;
            // find untill startItem
            result = mRoot->Find (inPred, startItem, &active);
        }
        QModelIndex modelIndex;
        if (result)  {
            modelIndex = createIndex (result->Index (), 0, const_cast <ObjectItem*> (result));
        }
        return modelIndex;
    }

} // namespace conan

#endif //_OBJECTMODEL__09_12_08__09_19_24__H_
