#ifndef CMAPOSMTYPE_H
#define CMAPOSMTYPE_H

#include <QString>

class CMapOSMType
{
    public:
        CMapOSMType();
        CMapOSMType(QString title, QString path);
        void setBuiltin(QString key);
        void setEnabled(bool state);
        bool isBuiltin();
        bool isEnabled();
        QString title;
        QString path;
        QString key;

    private:
        bool builtin;
        bool enabled;
};
#endif                           // CMAPOSMTYPE_H
