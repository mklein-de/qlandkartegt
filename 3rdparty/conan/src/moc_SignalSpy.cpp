/****************************************************************************
** Meta object code from reading C++ file 'SignalSpy.h'
**
** Created: Thu 31. Mar 20:33:34 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "SignalSpy.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SignalSpy.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_conan__SignalLogger[] = {

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
      25,   21,   20,   20, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_conan__SignalLogger[] = {
    "conan::SignalLogger\0\0msg\0SignalSpyLog(QString)\0"
};

const QMetaObject conan::SignalLogger::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_conan__SignalLogger,
      qt_meta_data_conan__SignalLogger, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &conan::SignalLogger::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *conan::SignalLogger::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *conan::SignalLogger::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_conan__SignalLogger))
        return static_cast<void*>(const_cast< SignalLogger*>(this));
    return QObject::qt_metacast(_clname);
}

int conan::SignalLogger::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: SignalSpyLog((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void conan::SignalLogger::SignalSpyLog(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
