#include "nixchartview.h"

NixChartView::NixChartView(QWidget *parent) :
    QChartView(parent)
{
    setContentsMargins(0,0,0,0);
}

NixChartView::NixChartView(QChart *chart, QWidget *parent) :
    QChartView(chart, parent)
{
    setContentsMargins(0,0,0,0);
}

/*
auto const widgetPos = event->localPos();
auto const scenePos = mapToScene(QPoint(static_cast<int>(widgetPos.x()), static_cast<int>(widgetPos.y())));
auto const chartItemPos = chart()->mapFromScene(scenePos);
auto const valueGivenSeries = chart()->mapToValue(chartItemPos);
*/
