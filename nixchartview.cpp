#include "nixchartview.h"

NixChartView::NixChartView(QWidget *parent) :
    QChartView(parent)
{
    setContentsMargins(0,0,0,0);
}


NixChartView::NixChartView(QChart *chart, QWidget *parent) :
    QChartView(chart, parent)
{
    this->setRenderHint(QPainter::Antialiasing);
    setContentsMargins(0,0,0,0);
    /*
     * setRubberBand(QtCharts::QChartView::RubberBand::RectangleRubberBand);
    this->chart()->legend()->setFont(QFont("Helvetica [Cronyx]", 9));
    this->chart()->legend()->setAlignment(Qt::AlignBottom);
    */
}


void NixChartView::set_series(QAbstractSeries *series) {
    this->chart()->removeAllSeries();
    this->chart()->addSeries(series);
}

void NixChartView::add_series(QAbstractSeries *series) {
    this->chart()->addSeries(series);
}

/*
auto const widgetPos = event->localPos();
auto const scenePos = mapToScene(QPoint(static_cast<int>(widgetPos.x()), static_cast<int>(widgetPos.y())));
auto const chartItemPos = chart()->mapFromScene(scenePos);
auto const valueGivenSeries = chart()->mapToValue(chartItemPos);
*/
