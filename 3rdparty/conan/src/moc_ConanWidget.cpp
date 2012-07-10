/****************************************************************************
** Meta object code from reading C++ file 'ConanWidget.h'
**
** Created: Thu 31. Mar 20:33:36 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ConanWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ConanWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_conan__ConanWidget[] = {

 // content:
       5,       // revision
       0,       // classname
       5,   14, // classinfo
      17,   24, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // classinfo: key, value
      25,   19,
      49,   33,
      76,   56,
     127,   84,
     139,  135,

 // signals: signature, parameters, type, tag, flags
     152,  148,  147,  147, 0x05,

 // slots: signature, parameters, type, tag, flags
     174,  147,  147,  147, 0x08,
     207,  186,  147,  147, 0x08,
     257,  147,  147,  147, 0x08,
     271,  147,  147,  147, 0x08,
     301,  288,  147,  147, 0x08,
     329,  147,  147,  147, 0x08,
     359,  147,  147,  147, 0x08,
     387,  381,  147,  147, 0x08,
     426,  147,  147,  147, 0x08,
     449,  147,  147,  147, 0x08,
     476,  381,  147,  147, 0x08,
     519,  147,  147,  147, 0x08,
     537,  381,  147,  147, 0x08,
     575,  147,  147,  147, 0x08,
     607,  147,  147,  147, 0x08,
     649,  625,  147,  147, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_conan__ConanWidget[] = {
    "conan::ConanWidget\0""1.0.2\0Version\0"
    "Elmar de Koning\0Author\0edekoning@gmail.com\0"
    "Contact\0http://sourceforge.net/projects/conanforqt\0"
    "Website\0GPL\0License\0\0msg\0SignalSpyLog(QString)\0"
    "SlotAbout()\0inCurrent,inPrevious\0"
    "SlotCurrentObjectChanged(QModelIndex,QModelIndex)\0"
    "SlotRefresh()\0SlotFindObject()\0"
    "inProxyIndex\0SlotFindMethod(QModelIndex)\0"
    "SlotFindDuplicateConnection()\0"
    "SlotDiscoverObjects()\0inPos\0"
    "SlotObjectContextMenuRequested(QPoint)\0"
    "SlotRemoveRootObject()\0"
    "SlotRemoveAllRootObjects()\0"
    "SlotConnectionContextMenuRequested(QPoint)\0"
    "SlotExportToXML()\0"
    "SlotSpiesContextMenuRequested(QPoint)\0"
    "SlotUpdateSignalLoggerOptions()\0"
    "SlotDeleteSpies()\0title,message,confirmed\0"
    "SlotRequestConfirmation(QString,QString,bool&)\0"
};

const QMetaObject conan::ConanWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_conan__ConanWidget,
      qt_meta_data_conan__ConanWidget, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &conan::ConanWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *conan::ConanWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *conan::ConanWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_conan__ConanWidget))
        return static_cast<void*>(const_cast< ConanWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int conan::ConanWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: SignalSpyLog((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: SlotAbout(); break;
        case 2: SlotCurrentObjectChanged((*reinterpret_cast< const QModelIndex(*)>(_a[1])),(*reinterpret_cast< const QModelIndex(*)>(_a[2]))); break;
        case 3: SlotRefresh(); break;
        case 4: SlotFindObject(); break;
        case 5: SlotFindMethod((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 6: SlotFindDuplicateConnection(); break;
        case 7: SlotDiscoverObjects(); break;
        case 8: SlotObjectContextMenuRequested((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 9: SlotRemoveRootObject(); break;
        case 10: SlotRemoveAllRootObjects(); break;
        case 11: SlotConnectionContextMenuRequested((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 12: SlotExportToXML(); break;
        case 13: SlotSpiesContextMenuRequested((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 14: SlotUpdateSignalLoggerOptions(); break;
        case 15: SlotDeleteSpies(); break;
        case 16: SlotRequestConfirmation((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        default: ;
        }
        _id -= 17;
    }
    return _id;
}

// SIGNAL 0
void conan::ConanWidget::SignalSpyLog(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
