#ifndef CMAPOSMTYPE_H
#define CMAPOSMTYPE_H

#include <QString>

class CMapOSMType
{
public:
    CMapOSMType(QString title, QString path);
    void setBuiltin(QString key);
    bool isBuiltin();
    QString title;
    QString path;
    QString key;

private:
    bool builtin;
};

#endif // CMAPOSMTYPE_H
