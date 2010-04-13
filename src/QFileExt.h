#pragma once

#include <QtCore/QFile>

class QFileExt : public QFile
{
    public:
        QFileExt(const QString &filename)
            : QFile(filename)
            , m_mapped(NULL)
            {}

        // data access function
        const char *data(qint64 offset)
        {
            if(!m_mapped)
                m_mapped = reinterpret_cast<const char*>(map(0, size()));
            return m_mapped + offset;
        }
    private:
        const char *m_mapped;
};
