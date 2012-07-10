/****************************************************************************
** Meta object code from reading C++ file 'ConnectionModel.h'
**
** Created: Thu 31. Mar 20:33:35 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ConnectionModel.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ConnectionModel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_conan__ConnectionModel[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      48,   24,   23,   23, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_conan__ConnectionModel[] = {
    "conan::ConnectionModel\0\0title,message,confirmed\0"
    "SignalRequestConfirmation(QString,QString,bool&)\0"
};

const QMetaObject conan::ConnectionModel::staticMetaObject = {
    { &QAbstractItemModel::staticMetaObject, qt_meta_stringdata_conan__ConnectionModel,
      qt_meta_data_conan__ConnectionModel, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &conan::ConnectionModel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *conan::ConnectionModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *conan::ConnectionModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_conan__ConnectionModel))
        return static_cast<void*>(const_cast< ConnectionModel*>(this));
    return QAbstractItemModel::qt_metacast(_clname);
}

int conan::ConnectionModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractItemModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: SignalRequestConfirmation((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void conan::ConnectionModel::SignalRequestConfirmation(const QString & _t1, const QString & _t2, bool & _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_conan__ConnectionFilterProxyModel[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      44,   35,   34,   34, 0x0a,
      78,   35,   34,   34, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_conan__ConnectionFilterProxyModel[] = {
    "conan::ConnectionFilterProxyModel\0\0"
    "inEnable\0SlotEnableInactiveFiltering(bool)\0"
    "SlotEnableInheritedFiltering(bool)\0"
};

const QMetaObject conan::ConnectionFilterProxyModel::staticMetaObject = {
    { &QSortFilterProxyModel::staticMetaObject, qt_meta_stringdata_conan__ConnectionFilterProxyModel,
      qt_meta_data_conan__ConnectionFilterProxyModel, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &conan::ConnectionFilterProxyModel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *conan::ConnectionFilterProxyModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *conan::ConnectionFilterProxyModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_conan__ConnectionFilterProxyModel))
        return static_cast<void*>(const_cast< ConnectionFilterProxyModel*>(this));
    return QSortFilterProxyModel::qt_metacast(_clname);
}

int conan::ConnectionFilterProxyModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QSortFilterProxyModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: SlotEnableInactiveFiltering((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: SlotEnableInheritedFiltering((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
