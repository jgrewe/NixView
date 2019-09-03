#ifndef NIXCHARTVIEW_H
#define NIXCHARTVIEW_H

#include <QObject>
#include <QtCharts/QChartView>

QT_CHARTS_USE_NAMESPACE

class NixChartView : public QChartView
{

public:
    NixChartView(QWidget *parent = 0);

    NixChartView(QChart *chart, QWidget *parent = 0);



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
