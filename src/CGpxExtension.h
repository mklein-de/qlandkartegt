//TODO: Includes & Abhngigkeiten
#ifndef CGPXEXTENSION_H
#define CGPXEXTENSION_H

#include <QMap>
#include <QDomElement>
#include <QDomNode>
#include <QString>
#include <QSet>
#include <QList>

//TODO: C: extensions auslesen und verarbeiten
class CGpxExtPt
{
    public:

        CGpxExtPt()              // der Default-Konstruktor
        {
        };
        ~CGpxExtPt()             // der Destruktor
        {
        };

                                 //Methode um die extensions aus dem xml file in eine values map zu packen
        void setValues(const QDomNode& parent);
        int getSize();
                                 //abfrage ber die anzahl der extension pro pt
        QString getName (const QString& val);
                                 //ausgabe des wertes der extension
        QString getValue (const QString& name);

                                 //deklaration der QMap values, die die extensions enthlt
        QMap<QString, QString> values;

    private:

};

class CGpxExtTr
{
    public:
        void addKey2List(const QDomNode& parent);
        QList<QString> list;     // Deklaration einer liste in die die QMap namen sollen
        int lsize;
        QSet<QString> set;       // Deklaration eines set in die die QMap namen sollen

    private:

};
#endif
