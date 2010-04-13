
#include "CTrackStatExtensionWidget.h"
#include "CPlot.h"
#include "CTrackDB.h"
#include "CTrack.h"
#include "IUnit.h"

#include <QtGui>

int ext_handler = 0;             //TODO: handler ext

CTrackStatExtensionWidget::CTrackStatExtensionWidget(type_e type, QWidget * parent)
: ITrackStat(type, parent)
, needResetZoom(true)
{

    connect(&CTrackDB::self(),SIGNAL(sigChanged()),this,SLOT(slotChanged()));
    connect(&CTrackDB::self(), SIGNAL(sigHighlightTrack(CTrack*)), this, SLOT(slotSetTrack(CTrack*)));

    slotChanged();
    plot->setLimits();
    plot->resetZoom();

}


CTrackStatExtensionWidget::~CTrackStatExtensionWidget()
{
}


                                 //TODO: hier wird gezeichnet
void CTrackStatExtensionWidget::slotChanged()
{
    track = CTrackDB::self().highlightedTrack();
    if(track.isNull())
    {
        plot->clear();
        return;
    }

    QPolygonF lineExt;
    QPolygonF marksExt;
    QPointF   focusExt;

    //QPointF(if type = eDist => distanz else timestamp (x-wert),	 speed * factor (y-wert) )
    //lineSpeed       << QPointF(type == eOverDistance ? trkpt->distance : (double)trkpt->timestamp, trkpt->speed * speedfactor);
    //lineAvgSpeed    << QPointF(type == eOverDistance ? trkpt->distance : (double)trkpt->timestamp, trkpt->avgspeed * speedfactor);

    QList<CTrack::pt_t>& trkpts = track->getTrackPoints();
    QList<CTrack::pt_t>::iterator trkpt = trkpts.begin();

    double val = 0;
                                 //Anzahl der Extensions
    num_of_ext =  track->tr_ext.set.toList().size();
    QString nam;

    if (num_of_ext != 0)
    {

                                 //Namen der Extensions
        names_of_ext = track->tr_ext.set.toList();

        //--- extensions den tabs zuordnen
        int e = ext_handler;
        if (e <= 0)              //sicherheit
        {
            e = 0;
        }
        else if (e > num_of_ext) {e = num_of_ext;}

        nam = names_of_ext[e];   //TODO: nam is name of extension nr e

        if (ext_handler == num_of_ext-1)    {ext_handler = 0;}
        else                                {ext_handler++;}
        //--- extensions den tabs zuordnen ^^

        plot->setXLabel(tr("time [h]"));
        plot->setYLabel(nam);

    }

    while(trkpt != trkpts.end())
    {
                                 //Wert einfgen
        QString val1 = trkpt->gpx_exts.getValue(nam);
        if (val1 == "")     {val = 0;}
        else                {val = val1.toDouble();}

        if(trkpt->flags & CTrack::pt_t::eDeleted)
        {
            ++trkpt; continue;
        }

        lineExt << QPointF((double)trkpt->timestamp, val);

        if(trkpt->flags & CTrack::pt_t::eSelected)
        {
            marksExt << QPointF((double)trkpt->timestamp, val);
        }

        if(trkpt->flags & CTrack::pt_t::eFocus)
        {
            focusExt = QPointF((double)trkpt->timestamp, val);
        }

        ++trkpt;
    }

    plot->newLine(lineExt,focusExt, nam);
    plot->newMarks(marksExt);

    plot->setLimits();
    if (needResetZoom)
    {
        plot->resetZoom();
        needResetZoom = false;
    }

}


void CTrackStatExtensionWidget::slotSetTrack(CTrack* track)
{
    needResetZoom = true;
}
