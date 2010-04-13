
#ifndef CTRACKSTATEXTENSIONWIDGET_H
#define CTRACKSTATEXTENSIONWIDGET_H

#include "ITrackStat.h"
#include "CTrackEditWidget.h"

extern int ext_handler; //TODO: Extension handler

class CTrackStatExtensionWidget : public ITrackStat
{
	Q_OBJECT;
    public:
        CTrackStatExtensionWidget(type_e type, QWidget * parent);
        virtual ~CTrackStatExtensionWidget();

    private slots:
        void slotChanged();
        void slotSetTrack(CTrack* track);

    private:
        bool needResetZoom;
		int num_of_ext;					//TODO: Anzahl der extensions
		QList<QString> names_of_ext;	//TODO: Namen der extensions


};
#endif                           //CTRACKSTATEXTENSIONWIDGET_H
