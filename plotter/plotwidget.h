#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include <QWidget>
#include <nix.hpp>
#include <QVariant>
#include <QModelIndex>
#include "plotter/plotter.h"
#include "views/MainViewWidget.hpp"
#include "utils/entitydescriptor.h"
#include "utils/datacontroller.h"


namespace Ui {
class PlotWidget;
}

class PlotWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlotWidget(QWidget *parent = 0);
    ~PlotWidget();

    bool canDraw() const;
    void dataSource(const EntityInfo &info);

    void clear();

    Plotter* processDataArray();
    void processMTag(const nix::MultiTag &mtag, nix::ndsize_t ref=0);
    void processTag(const nix::Tag &tag, nix::ndsize_t ref=0);
    void processFeature(const nix::Feature &feat, const nix::Tag & tag);
    //void processDataArray(const nix::Feature &feat, const nix::MultiTag & mtag);

public slots:
    void showMore();
    void savePlot();

    void resetView();
    // slots and signals for the scrollbars and the zoom slider:
    void hScrollBarPosChanged(int value);   // emit signal to be caught by the plotter.
    void vScrollBarPosChanged(int value);
    void sliderPosChanged(int value);

    void changeHScrollBarValue(QCPRange newRange, QCPRange completeRange); // react to signals from the plotter.
    void changeVScrollBarValue(QCPRange newRange, QCPRange completeRange);
    void changeSliderPos(QCPRange xNow, QCPRange xComplete);

signals:
    void hScrollBarToPlot(double); // signals for the plotter.
    void vScrollBarToPlot(double);
    void sliderToPlot(double);
    void resetViewToPlot();

private:
    Ui::PlotWidget *ui;
    EntityInfo data_src;
    Plotter *plot;
    QString text;
    double scrollFaktor;
    double zoomMax;

    bool checkPlottableDType(nix::DataType dtype) const;
    void deleteWidgetsFromLayout();
    void processItem();

    void draw1D(const nix::DataArray &array);
    void draw2D(const nix::DataArray &array);
    void drawMultiLine(const nix::DataArray &array);

    int sliderMapToValue(QCPRange current, QCPRange complete);
};

#endif // PLOTWIDGET_H
