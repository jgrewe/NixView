#ifndef NIXCHARTVIEW_H
#define NIXCHARTVIEW_H

#include <QObject>
#include <QtCharts/QChartView>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLineSeries>

QT_CHARTS_USE_NAMESPACE

class NixChartView : public QChartView
{

public:
    NixChartView(QWidget *parent = nullptr);

    NixChartView(QChart *chart, QWidget *parent = nullptr);

    void set_series(QAbstractSeries *series);
    void add_series(QAbstractSeries *series);

//private:


};
/*class ChartView : public QChartView
{
public:
    ChartView();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    QPointF mouseDownPos;
};

#endif // CHARTVIEW_H
*/
#endif // NIXCHARTVIEW_H
